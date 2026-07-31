plugins {
    id("com.android.application")
}

android {
    namespace = "com.zygisk.imgui"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.zygisk.imgui"
        minSdk = 24
        targetSdk = 34
        versionCode = 100
        versionName = "1.0.0"

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17 -fvisibility=hidden -fdata-sections -ffunction-sections")
                arguments("-DANDROID_STL=c++_static")
                abiFilters("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        debug {
            isMinifyEnabled = false
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    ndkVersion = "25.2.9519653"
}

// Magisk Module ZIP Packaging Task
tasks.register<Zip>("zipModule") {
    dependsOn("externalNativeBuildRelease")

    archiveFileName.set("zygisk_imgui_menu.zip")
    destinationDirectory.set(file("${rootDir}/out"))

    // Include base module properties
    from(file("${rootDir}/module")) {
        include("module.prop", "service.sh", "post-fs-data.sh", "system.prop")
    }

    // Include compiled libzygisk.so native libraries for each ABI
    val abis = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
    abis.forEach { abi ->
        val candidateDirs = listOf(
            file("${buildDir}/intermediates/stripped_native_libs/release/out/lib/$abi"),
            file("${buildDir}/intermediates/merged_native_libs/release/out/lib/$abi"),
            file("${buildDir}/intermediates/cmake/release/obj/$abi"),
            file("${buildDir}/intermediates/cxx/Release/*/obj/$abi"),
            file("${buildDir}/intermediates/cxx/RelWithDebInfo/*/obj/$abi")
        )

        val libFile = candidateDirs.map { file("${it.path}/libzygisk.so") }.firstOrNull { it.exists() }
        if (libFile != null) {
            from(libFile) {
                into("zygisk/$abi")
                rename { "libzygisk.so" }
            }
            from(libFile) {
                into("zygisk")
                rename { "$abi.so" }
            }
        }
    }
}

tasks.named("assembleRelease") {
    finalizedBy("zipModule")
}
