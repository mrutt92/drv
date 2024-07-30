find_package(DRV REQUIRED)
project(drvr_example_project)
cmake_minimum_required(VERSION 3.25.0)

add_drvx_executable(
  command_processor
  command_processor.cpp
  )

drvx_target_link_libraries(
  command_processor
  PRIVATE
  pandocommand
  )

add_drvr_executable(
  drvr-example
  hello.c
  )

add_drvr_run_target(
  run-drvr-example
  drvr-example
  )

set_drvr_run_target_properties(
  run-drvr-example
  PROPERTIES
  DRV_MODEL_NUM_PXN      1
  DRV_MODEL_PXN_PODS     1
  DRV_MODEL_POD_CORES    1
  DRV_MODEL_CORE_THREADS 1
  )


add_drvr_run_target_with_command_processor(
  run-drvr-example-with-command-processor
  drvr-example
  command_processor
  )


set_drvr_run_target_properties(
  run-drvr-example-with-command-processor
  PROPERTIES
  DRV_MODEL_NUM_PXN      1
  DRV_MODEL_PXN_PODS     1
  DRV_MODEL_POD_CORES    1
  DRV_MODEL_CORE_THREADS 1
  )
