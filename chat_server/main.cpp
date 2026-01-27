#include "pch.h"
#include "startup_validator.h"
#include "utility/file/path.h"
#include "utility/exception/exception_handler.h"
#include "utility/log/static_logger.h"
#include "utility/log.h"
#include "chat_server/configs/server_setting.h"
#include "chat_server/chat_server.h"
#include "chat_server/chat/chat_room_selector.h"
#include "configs/config_loader.h"

#include "database/database_service.h"
#include "handler/packet_handler_registry.h"

using namespace dev;
using namespace chat_server;

static bool InitializeEnvironment() {
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "");
    utility::Path::SetCurrentPath(utility::Path::GetExeFileName());

    static const std::wstring kServerName = L"ChatServer";
    if (ERROR_SUCCESS != utility::ExceptionHandler::Start(
        kServerName,
        static_cast<MINIDUMP_TYPE>(
            MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithDataSegs |
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithThreadInfo))) {
        SPDLOG_ERROR("[ChatServer] Fail to start exception handler.");
        return false;
    }
    return true;
}

static std::unique_ptr<utility::ILogger> CreateLogger(
    const ServerSetting& server_setting,
    const utility::StaticLogger::LogLevelInfo& log_level_info
) {
    std::unique_ptr<utility::ILogger> logger
        = std::make_unique<utility::StaticLogger>("log/app.log", log_level_info, server_setting.log_info.max_file_count);
    if (!logger->Initialize()) {
        return nullptr;
    }
    return logger;
}

static void RunLogicThread(
    std::vector<std::thread>& threads,
    boost::asio::io_context& logic_io_context,
    boost::asio::io_context& network_io_context
) {
    threads.emplace_back([&logic_io_context, &network_io_context] {
        LOG_INFO("[ChatServer] Start logic_thread.");
        // 로직 스레드 시작
        while (!network_io_context.stopped()) {
            auto count = logic_io_context.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        LOG_INFO("[ChatServer] network_io_context stopped.");

        // 로직 스레드 종료 대기
        while (logic_io_context.poll() > 0) {
        }
        LOG_INFO("[ChatServer] logic_io_context Stopped.");
        logic_io_context.stop();
    });

}

static void RunNetworkThreads(
    std::vector<std::thread>& threads,
    boost::asio::io_context& network_io_context,
    int32_t run_thread_count
) {
    //const int32_t network_thread_count = std::thread::hardware_concurrency();
    LOG_INFO("[ChatServer] Start. network_thread_count: {}", run_thread_count);
    for (size_t index = 0; index < run_thread_count; ++index) {
        threads.emplace_back([&network_io_context] {
            network_io_context.run();
        });
    }
    network_io_context.run();
}

int main(int argc, char* argv[]) {
    // 환경 설정
    if (!InitializeEnvironment()) {
        SPDLOG_ERROR("[ChatServer] Fail to load InitializeEnvironment.");
        return EXIT_FAILURE;
    }

    // 설정 파일 로드
    if (!ConfigLoader::LoadServerSetting("config/server_setting.json")) {
        SPDLOG_ERROR("[ChatServer] Fail to load config.");
        return EXIT_FAILURE;
    }

    // 로거 생성
    const auto& server_setting = ServerConfig::server_setting();
    auto logger = CreateLogger(
        server_setting,
        {
            .console_level = spdlog::level::level_enum::trace,
            .file_level = spdlog::level::level_enum::trace
        });

    boost::asio::thread_pool db_thread_pool(server_setting.sql_info.thread_pool_count);
    boost::asio::io_context network_io_context;
    boost::asio::io_context logic_io_context;

    // 데이터베이스 서비스 시작
    auto db_service = std::make_unique<database::DatabaseService>(*logger, db_thread_pool, server_setting.sql_info);
    if (!db_service->Start()) {
        LOG_ERROR("[ChatServer] Fail to start db_service.");
        return EXIT_FAILURE;
    }

    // 서버 시작 조건 체크
    if (!StartupValidator(*logger, server_setting, *db_service).Validate()) {
        LOG_ERROR("[ChatServer] Startup validation failed.");
        return EXIT_FAILURE;
    }

    // 종료 등록
    boost::asio::signal_set signals(network_io_context, SIGINT, SIGTERM);
    signals.async_wait(
        [&](boost::system::error_code const&, int) {
        network_io_context.stop();
    });

    // 패킷 핸들러 등록
    auto handler_registry = std::make_unique<PacketHandlerRegistry>(*logger);
    if (!handler_registry->Initialize()) {
        LOG_ERROR("[ChatServer] Fail to initialize PacketHandlerRegistry.");
        return EXIT_FAILURE;
    }

    // 네트워크 의존성 설정
    ChatServer::NetworkDependency network_dependency{
        .io_context = network_io_context,
        .endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("0.0.0.0"), ServerConfig::server_setting().server_info.port),
        .logger = *logger,
        .packet_handler = handler_registry->packet_handler()
    };

    // 채팅 서버 정하는 기준 설정
    auto chat_room_selector = std::make_unique<MaxCapacitySelector>(
        logic_io_context,
        MaxCapacitySelector::Setting{ .max_room_count = 4, .max_capacity_per_room = 5000 }
    );
    // 채팅 서버 시작
    const auto chat_server = std::make_unique<ChatServer>(network_dependency, *db_service, *chat_room_selector);
    chat_server->Start();

    // I/O 작업 시작
    std::vector<std::thread> threads;
    RunLogicThread(threads, logic_io_context, network_io_context);
    RunNetworkThreads(threads, network_io_context, std::thread::hardware_concurrency());

    // 스레드 조인
    for (auto& thread : threads) {
        thread.join();
    }
    // DB 서비스 종료
    db_service->Stop();

    LOG_INFO("[ChatServer] End");
    return EXIT_SUCCESS;
}
