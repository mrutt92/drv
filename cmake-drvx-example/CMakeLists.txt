add_library(example SHARED main.cpp)
target_link_libraries(example PRIVATE drvapi)

function (add_drvx_run_target name)
  set(ARGS ${ARGV})
  list(POP_FRONT ARGS)
  list(JOIN ARGS "_" ARGSTR)
  add_custom_target(run_${name}_${ARGSTR}
    COMMAND
    PYTHONPATH=${PROJECT_SOURCE_DIR}/py:${PROJECT_SOURCE_DIR}/tests
    $<TARGET_FILE:SST::SST> ${PROJECT_SOURCE_DIR}/tests/PANDOHammerDrvX.py --
    --core-threads=1
    --pod-cores=1
    --pxn-pods=1
    --num-pxn=1
    $<TARGET_FILE:${name}>
    ${ARGS}
    DEPENDS ${name}
    )
endfunction()

add_drvx_run_target(example hello)
add_drvx_run_target(example world)
add_drvx_run_target(example hello world)
