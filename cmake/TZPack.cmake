#common info
SET(CPACK_PACKAGE_VENDOR "EPAM Systems")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Crossplatform library for DT locale convertations")
set(CPACK_GENERATOR "TGZ")
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
INSTALL(FILES ../include/libtz/dt.h DESTINATION include/libtz)
INSTALL(FILES ../include/libtz/dt_precise.h DESTINATION include/libtz)
INSTALL(FILES ../include/libtz/dt_types.h DESTINATION include/libtz)
INCLUDE(CPack)
