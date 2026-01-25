#include "pch.h"
#include "utility/log/dummy_logger.h"

#include "public_chat/chat_handler.h"
#include "dummy_client/dummy_session.h"

int main(int argc, char* argv[]) {
    using namespace dev;
    // 필요한 경우 UTF-8 문자열 처리
    std::locale::global(std::locale(""));

    std::unique_ptr<utility::ILogger> logger = std::make_unique<utility::DummyLogger>();
    if (!logger->Initialize()) {
        return EXIT_FAILURE;
    }
    
    logger->LogDebug("Start.");

    boost::asio::io_context io_context;

    auto packet_handler = std::make_unique<network::PacketHandler>(*logger);

    if (!dummy_client::ChatHandler::Register(*packet_handler)) {
        return EXIT_FAILURE;
    }

    const auto client_endpoint
        = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 3200);

    std::vector<std::shared_ptr<dummy_client::DummySession>> dummy_sessions;
    for (int32_t index = 0; index < 100; ++index) {
        boost::asio::ip::tcp::socket socket(io_context);
        auto session =
            std::make_shared<dummy_client::DummySession>(io_context, std::move(socket), *logger, *packet_handler);
        session->Connect(client_endpoint);

        dummy_sessions.emplace_back(session);
    }

    const int32_t thread_count = 2;
    std::vector<std::thread> threads;
    for (size_t index = 1; index < thread_count; ++index) {
        threads.emplace_back([&io_context, thread_index = index] {
            io_context.run();
        });
    }
    io_context.run();

    for (auto& thread : threads) {
        thread.join();
    }

    return EXIT_SUCCESS;
}
