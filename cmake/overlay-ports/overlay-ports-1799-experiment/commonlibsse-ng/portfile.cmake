vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO alandtse/CommonLibSSE-NG
        REF v6.6.0
        SHA512 7b46371745d592cd1cec3d2f4cf9e1321dd8805de9895141b93c9dfc06fb82e3b10e8f4fd304342c3446ae3bd6f77ee22848edd1e639866fe6335cc65c8f47b7
        HEAD_REF main
)
vcpkg_configure_cmake(
        SOURCE_PATH "${SOURCE_PATH}"
        PREFER_NINJA
        OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on -DENABLE_SKYRIM_VR=off
)
vcpkg_install_cmake()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()
file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")
file(
        INSTALL "${SOURCE_PATH}/licenses/LICENSE-MIT"
        DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
        RENAME copyright)