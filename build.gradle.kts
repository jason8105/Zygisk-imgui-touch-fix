plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imguitouchfix"
    compileSdk = 33

    defaultConfig {
        minSdk = 26
        targetSdk = 33

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
        
        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17", "-frtti", "-fexceptions")
                arguments("-DANDROID_STL=c++_shared")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.6.1")
}

// Task to assemble final flashable Magisk module zip containing compiled Zygisk binaries
tasks.register("buildMagiskModuleZip", Zip::class) {
    dependsOn("assembleRelease")
    archiveFileName.set("zygisk-imgui-touch-fix-release.zip")
    destinationDirectory.set(file("$buildDir/outputs/magisk"))

    from("$rootDir/module.prop")
    from("$rootDir/customize.sh")

    // Map compiled shared libraries into the standard Zygisk module directory layout
    into("zygisk/arm64-v8a") {
        from("$buildDir/intermediates/cmake/release/obj/arm64-v8a/libzygisk.so")
    }
    into("zygisk/armeabi-v7a") {
        from("$buildDir/intermediates/cmake/release/obj/armeabi-v7a/libzygisk.so")
    }
    into("zygisk/x86") {
        from("$buildDir/intermediates/cmake/release/obj/x86/libzygisk.so")
    }
    into("zygisk/x86_64") {
        from("$buildDir/intermediates/cmake/release/obj/x86_64/libzygisk.so")
    }
}
