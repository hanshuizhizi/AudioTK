
create_Python_test(PySpecial ${CMAKE_CURRENT_LIST_DIR})

file(GLOB entries *.py)
add_custom_target(PySpecial_test SOURCES ${entries})
set_target_properties (PySpecial_test PROPERTIES
    FOLDER Tests/Python
)
