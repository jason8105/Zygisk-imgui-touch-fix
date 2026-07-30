plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.example.zygisk_imgui"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.example.zygisk_imgui"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden")
                arguments("-DANDROID_STL=c++_static")
                abiFilters("arm64-v8a", "armeabi-v7a")
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
            path = file("src/main/jni/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

// Custom task to package the Magisk Module
tasks.register<Zip>("packageMagiskModule") {
    dependsOn("externalNativeBuildRelease")
    
    archiveFileName.set("Zygisk-ImGui-Universal.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/magisk"))

    // 1. Include module metadata
    from("src/main/root") {
        include("module.prop")
        include("post-fs-data.sh")
        include("service.sh")
    }

    // 2. Include the compiled Zygisk libraries in the correct structure
    val abiMap = mapOf("arm64-v8a" to "arm64-v8a", "armeabi-v7a" to "armeabi-v7a")
    abiMap.forEach { (abi, folder) ->
        from(layout.buildDirectory.dir("intermediates/cmake/release/obj/$abi")) {
            include("libzygisk.so")
            into("zygisk/$folder")
        }
    }
}
