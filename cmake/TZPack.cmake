set(CPACK_PACKAGE_VENDOR "EPAM Systems")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Crossplatform library for date time manipulation and locale convertations")
set(CPACK_GENERATOR "TGZ")
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
install(FILES ../include/libdt/dt.h DESTINATION include/libdt)
install(FILES ../include/libdt/dt_precise.h DESTINATION include/libdt)
install(FILES ../include/libdt/dt_types.h DESTINATION include/libdt)
include(CPack)
