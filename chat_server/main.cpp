#include "pch.h"
#include "utility/file/path.h"
#include "utility/exception/exception_handler.h"
#include "utility/file/file_loader.h"
#include "utility/log/static_logger.h"
#include "utility/system_status.h"
#include "utility/string/string_converter.h"
#include "utility/log.h"
#include "chat_server/configs/server_setting.h"
#include "chat_server/chat_server.h"
#include "chat_server/chat/chat_room_selector.h"
#include "chat_server/chat/chat_handler.h"
#include "chat_server/user_detail/user_handler.h"

#include <boost/mysql/connection_pool.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/pool_params.hpp>
#include <boost/mysql/static_results.hpp>
#include <boost/mysql/with_params.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/cancel_after.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/write.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/system/error_code.hpp>
#include <boost/mysql/static_results.hpp>
#include <boost/describe/class.hpp>
using namespace dev;

static bool LoadConfig() {
    try {
        const std::string server_setting_json = utility::FileLoader("config/server_setting.json").ReadFileToString();
        if (!chat_server::ServerConfig::Load(server_setting_json)) {
            return false;
        }
        SPDLOG_INFO("[Server] port: {}, processor_multiplier: {}",
            chat_server::ServerConfig::server_setting().server_info.port,
            chat_server::ServerConfig::server_setting().server_info.processor_multiplier);
    } catch (const std::runtime_error& e) {
        SPDLOG_ERROR("LoadConfig exception: {}", e.what());
        return false;
    }
    return true;
}

static std::unique_ptr<utility::ILogger> CreateLogger(
    const utility::StaticLogger::LogLevelInfo& log_level_info,
    const chat_server::ServerSetting& server_setting
) {
    std::unique_ptr<utility::ILogger> logger
        = std::make_unique<utility::StaticLogger>("log/app.log", log_level_info, server_setting.log_info.max_file_count);
    if (!logger->Initialize()) {
        return nullptr;
    }
    return logger;
}

static bool RegisterHandler(network::PacketHandler& packet_handler) {
    // 유저 핸들러 등록
    if (!chat_server::UserHandler::Register(packet_handler)) {
        return false;
    }
    // 채팅 핸들러 등록
    if (!chat_server::ChatHandler::Register(packet_handler)) {
        return false;
    }
    return true;
}

struct User {
    uint64_t user_uid;
    std::string nickname;
};
BOOST_DESCRIBE_STRUCT(User, (), (user_uid, nickname))

using boost::asio::awaitable;
using boost::asio::use_awaitable;
boost::asio::awaitable<User> Test(boost::mysql::connection_pool& pool)
{
    // 1. 풀에서 연결 객체를 빌려옵니다 (pooled_connection).
    // 이 객체는 범위를 벗어나면 자동으로 풀로 반환됩니다.
    boost::mysql::pooled_connection conn = co_await pool.async_get_connection(use_awaitable);

    // 2. 실행할 쿼리와 데이터를 준비합니다.
    boost::mysql::static_results<User> result;

    // 3. 쿼리 실행 (예: 특정 ID의 유저 이름 조회)
    // conn.operator*()를 통해 실제 connection 객체에 접근합니다.

    uint64_t user_uid = 51;
    //std::string tag = "[CheckStat] combat_point: 396318887155, ";
    co_await conn->async_execute(
        boost::mysql::with_params("SELECT * FROM user WHERE user_uid = {}", user_uid),
        result,
        use_awaitable
    );

    std::cout <<"1"<<std::endl;
    LOG_DEBUG("called Test");
    // 4. 결과 처리
    
    User u{};
    if (result.has_value()) {
        u = result.rows()[0]; // 첫 번째 행을 User 객체로 바로 가져옴
    }

    co_return u;
}


int main(int argc, char* argv[]) {
    // 필요한 경우 UTF-8 문자열 처리
    //std::locale::global(std::locale(""));
    static const std::wstring kServerName = L"ChatServer";
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "");
    // 현재 폴더로 지정
    utility::Path::SetCurrentPath(utility::Path::GetExeFileName());

    if (ERROR_SUCCESS != utility::ExceptionHandler::Start(
        kServerName,
        static_cast<MINIDUMP_TYPE>(
            MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithDataSegs |
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithThreadInfo))) {
        SPDLOG_ERROR("[Server] Fail to start exception handler.");
        return EXIT_FAILURE;
    }

    if (!LoadConfig()) {
        SPDLOG_ERROR("[Server] Fail to load config.");
        return EXIT_FAILURE;
    }

    const auto& server_setting = chat_server::ServerConfig::server_setting();

    const utility::StaticLogger::LogLevelInfo level_info{
       .console_level = spdlog::level::level_enum::trace,
       .file_level = spdlog::level::level_enum::trace
    };
    const std::unique_ptr<utility::ILogger> logger = CreateLogger(level_info, server_setting);
    if (logger == nullptr) {
        SPDLOG_ERROR("[Server] Fail to create logger.");
        return EXIT_FAILURE;
    }

    if (utility::PortInUse(server_setting.server_info.port)) {
        LOG_ERROR("[Server] Port already in use. port: {}", server_setting.server_info.port);
        return EXIT_FAILURE;
    }

    boost::asio::io_context network_io_context;
    boost::asio::io_context logic_io_context;

    // 종료 등록
    boost::asio::signal_set signals(network_io_context, SIGINT, SIGTERM);
    signals.async_wait(
        [&](boost::system::error_code const&, int) {
        network_io_context.stop();
    });

    // 채팅 서버 네트워크 설정
    const auto server_endpoint
        = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("0.0.0.0"), chat_server::ServerConfig::server_setting().server_info.port);
    auto packet_handler = std::make_unique<network::PacketHandler>(*logger);
    chat_server::ChatServer::NetworkDependency network_dependency{
        .io_context = network_io_context,
        .endpoint = server_endpoint,
        .logger = *logger,
        .packet_handler = *packet_handler
    };

    // 채팅서버 핸들러 등록
    if (!RegisterHandler(*packet_handler)) {
        return EXIT_FAILURE;
    }

    // 채팅 서버 시작
    auto chat_room_selector = std::make_unique<chat_server::MaxCapacitySelector>(
        logic_io_context,
        chat_server::MaxCapacitySelector::Setting{ .max_room_count = 4, .max_capacity_per_room = 5000 }
    );
    const auto chat_server = std::make_unique<chat_server::ChatServer>(network_dependency, *chat_room_selector);
    chat_server->Start();

    const int32_t processor_count = std::thread::hardware_concurrency();
    const int32_t network_thread_count = processor_count;

    LOG_INFO("[ChatServer] Start. thread_count: {}", network_thread_count);

    using namespace boost;

    mysql::pool_params params{
        .username = "",
        .password = "",
        .database = "",
        .thread_safe = true
    };
    params.server_address.emplace_host_and_port("");

    boost::asio::thread_pool db_thread_pool(4);

    mysql::connection_pool db_connection_pool(db_thread_pool, std::move(params));
    db_connection_pool.async_run(asio::detached);


    for (int32_t index = 0; index < 10000; ++index) {
        boost::asio::co_spawn(
            db_connection_pool.get_executor(),
            [&db_connection_pool] { return Test(db_connection_pool); },
            // 세 번째 인자에 User를 인자로 받는 람다를 넣습니다.
            [](std::exception_ptr e, User user) {
                if (!e) {
                    LOG_INFO("user_uid: {}", user.user_uid);
                    // 여기서 user 객체 사용
                    // 예: 공유 컨테이너에 담거나 로그 출력
                }
            }
        );

        // 받은 데이터 사용
        //ProcessUser(user);
        /*
        boost::asio::co_spawn(
            db_connection_pool.get_executor(), 
            [ &db_connection_pool ] {
                return Test(db_connection_pool);
            }, 
            boost::asio::detached
        );
        */
    }

    std::vector<std::thread> threads;

    // 반드시 필요
   

    threads.emplace_back([&logic_io_context, &network_io_context] {
        LOG_INFO("[ChatServer] Start logic_thread.");
        // 로직 스레드 시작
        while (!network_io_context.stopped()) {
            auto count = logic_io_context.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        LOG_INFO("[ChatServer] network_io_context stopped.");

        // network 멈춘 뒤 남은 logic 처리
        while (logic_io_context.poll() > 0) {
        }
        LOG_INFO("[ChatServer] logic_io_context Stopped.");
        logic_io_context.stop();
    });

    for (size_t index = 0; index < network_thread_count; ++index) {
        threads.emplace_back([&network_io_context] {
            network_io_context.run();
        });
    }
    network_io_context.run();

    for (auto& thread : threads) {
        thread.join();
    }
    db_thread_pool.stop();
    db_thread_pool.join(); // 여기서 DB 풀 스레드들을 정리합니다
    LOG_INFO("[ChatServer] End");
    return EXIT_SUCCESS;
}
