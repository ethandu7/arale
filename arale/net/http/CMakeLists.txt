
set(http_SRCS
	HttpContext.cc
	)
	
add_library(arale_http ${http_SRCS})

target_link_libraries(arale_http arale_net)

