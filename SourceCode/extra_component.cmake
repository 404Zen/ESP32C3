
add_compile_options(-fdiagnostics-color=always)

list(APPEND EXTRA_COMPONENT_DIRS 
                                "$ENV{IOT_SOLUTION_PATH}/components/button"
                                "$ENV{IDF_PATH}/examples/system/console/advanced/components"
                                )

