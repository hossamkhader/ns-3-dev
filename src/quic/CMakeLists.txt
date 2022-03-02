build_lib(
  LIBNAME quic
  SOURCE_FILES
  	helper/quic-helper.cc
  	helper/quic-client-server-helper.cc
  	helper/quic-echo-helper.cc
    model/quic-congestion-ops.cc
	model/quic-header.cc
	model/quic-l4-protocol.cc
	model/quic-l5-protocol.cc
	model/quic-socket-base.cc
	model/quic-socket.cc
	model/quic-socket-factory.cc
	model/quic-socket-rx-buffer.cc
	model/quic-socket-tx-buffer.cc
	model/quic-stream-base.cc
	model/quic-stream.cc
	model/quic-stream-rx-buffer.cc
	model/quic-stream-tx-buffer.cc
	model/quic-subheader.cc
	model/quic-transport-parameters.cc
	model/quic-client.cc
	model/quic-echo-client.cc
	model/quic-echo-server.cc
	model/quic-server.cc
	model/quic-socket-tx-scheduler.cc
	model/quic-socket-tx-edf-scheduler.cc
  HEADER_FILES
  	helper/quic-helper.h
  	helper/quic-client-server-helper.h
  	helper/quic-echo-helper.h
  	model/quic-congestion-ops.h
	model/quic-header.h
	model/quic-l4-protocol.h
	model/quic-l5-protocol.h
	model/quic-socket-base.h
	model/quic-socket-factory.h
	model/quic-socket.h
	model/quic-socket-rx-buffer.h
	model/quic-socket-tx-buffer.h
	model/quic-stream-base.h
	model/quic-stream.h
	model/quic-stream-rx-buffer.h
	model/quic-stream-tx-buffer.h
	model/quic-subheader.h
	model/quic-transport-parameters.h
	model/quic-client.h
	model/quic-echo-client.h
	model/quic-echo-server.h
	model/quic-server.h
	model/quic-socket-tx-scheduler.h
	model/quic-socket-tx-edf-scheduler.h
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libapplications}
    ${libflow-monitor}
    ${libpoint-to-point}
  TEST_SOURCES test/quic-header-test.cc
               test/quic-rx-buffer-test.cc
               test/quic-tx-buffer-test.cc
)

	
	
  	
    
