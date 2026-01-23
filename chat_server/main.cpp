#include "pch.h"
#include "utility/file/path.h"
#include "utility/exception/exception_handler.h"
#include "utility/file/file_loader.h"
#include "utility/log/static_logger.h"
#include "utility/system_status.h"

#include "utility/log.h"
#include "chat_server/configs/server_setting.h"
#include "chat_server/chat_server.h"
#include "chat_server/chat/chat_room_selector.h"
#include "chat_server/chat/chat_handler.h"
#include "chat_server/user_detail/user_handler.h"

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

std::shared_ptr<boost::mysql::connection_pool> CreateDBConnectionPool(
    boost::asio::thread_pool& db_thread_pool,
    const chat_server::ServerSetting& server_setting
) {
    boost::mysql::pool_params params{
        .username = "",
        .password = "",
        .database = "",
        .thread_safe = true
    };
    params.server_address.emplace_host_and_port("");
    auto db_connection_pool = std::make_shared<boost::mysql::connection_pool>(db_thread_pool, std::move(params));
    BOOST_ASSERT(db_connection_pool != nullptr);
    return db_connection_pool;
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
    boost::asio::thread_pool db_thread_pool(4);

    auto db_conn_pool = CreateDBConnectionPool(db_thread_pool, server_setting);
    db_conn_pool->async_run(boost::asio::detached);

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
    const auto chat_server = std::make_unique<chat_server::ChatServer>(network_dependency, db_conn_pool, *chat_room_selector);
    chat_server->Start();

    const int32_t processor_count = std::thread::hardware_concurrency();
    const int32_t network_thread_count = processor_count;

    LOG_INFO("[ChatServer] Start. thread_count: {}", network_thread_count);

    std::vector<std::thread> threads;

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
