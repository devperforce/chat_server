#pragma once

#include <vector>
#include <string>
#include <locale>
#include <ranges>
#include <expected>

#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/describe/class.hpp>
#include <boost/system/error_code.hpp>
#include <boost/locale.hpp>

#include <boost/mysql/connection_pool.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/pool_params.hpp>
#include <boost/mysql/static_results.hpp>
#include <boost/mysql/with_params.hpp>
