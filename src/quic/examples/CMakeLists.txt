set (base_examples
     quic-tester
     quic-tester-streams
     quic-variants-comparison-bulksend
     quic-variants-comparison
     quic-pacing
     )
foreach (
  example
  $ {base_examples}
  )
build_lib_example (
  NAME $ {example}
  SOURCE_FILES $ {example}.cc
  LIBRARIES_TO_LINK
  $ {libquic}
  $ {libinternet}
  $ {libapplications}
  $ {libflow-monitor}
  $ {libpoint-to-point}
  )
endforeach ()
