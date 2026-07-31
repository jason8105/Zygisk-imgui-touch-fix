plugins {
    id("com.android.application")
}

android {
    namespace = "com.zygisk.menu"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.zygisk.menu"
        minSdk = 24
        targetSdk = 34
        versionCode = 100
        versionName = "1.0.0"

        ndk {
            abiFilters.addAll(setOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64"))
        }

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17 -frtti -fexceptions")
                arguments("-DANDROID_STL=c++_static")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

tasks.register<Zip>("packageMagiskModule") {
    dependsOn("externalNativeBuildRelease")

    archiveFileName.set("zygisk-module.zip")
    destinationDirectory.set(file("${project.rootDir}/out"))

    // Base module files
    from("${project.rootDir}/module") {
        include("module.prop", "customize.sh", "service.sh", "system/**", "sepolicy.rule")
    }

    // Pack compiled NDK binaries into zygisk/<abi>/libzygisk.so and zygisk/<abi>.so
    val abiList = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
    for (abi in abiList) {
        val strippedLib = file("${project.buildDir}/intermediates/stripped_native_libs/release/out/lib/$abi/libzygisk.so")
        val mergedLib = file("${project.buildDir}/intermediates/merged_native_libs/release/out/lib/$abi/libzygisk.so")
        val cmakeLib = file("${project.buildDir}/intermediates/cmake/release/obj/$abi/libzygisk.so")

        into("zygisk/$abi") {
            from(strippedLib, mergedLib, cmakeLib) {
                rename { "libzygisk.so" }
            }
        }
        into("zygisk") {
            from(strippedLib, mergedLib, cmakeLib) {
                rename { "$abi.so" }
            }
        }
    }
}

tasks.named("assemble") {
    finalizedBy("packageMagiskModule")
}
