find_package(DRV REQUIRED)
project(drvr_example_project)
cmake_minimum_required(VERSION 3.25.0)

add_drvr_executable(
  drvr-example
  hello.c
  )

add_drvr_run_target(
  run-drvr-example-hello
  drvr-example
  )


set_drvr_run_target_properties(
  run-drvr-example-hello
  PROPERTIES
  DRV_MODEL_NUM_PXN      1
  DRV_MODEL_PXN_PODS     1
  DRV_MODEL_POD_CORES    1
  DRV_MODEL_CORE_THREADS 1
  )
