plugins {
    id("com.android.library")
}

android {
    namespace = "com.zy.cheats"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_STL=c++_shared"
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
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

tasks.register("buildMagiskZip", Zip::class) {
    dependsOn("assembleRelease")
    archiveFileName.set("zyCheats-Magisk-Module.zip")
    destinationDirectory.set(file("$buildDir/outputs/magisk"))

    from("src/main/magisk") {
        include("module.prop")
        include("customize.sh")
        include("post-fs-data.sh")
        include("service.sh")
    }
    
    // Package compiled native libraries into Zygisk structure
    from("$buildDir/intermediates/cmake/release/obj") {
        include("**/zyCheats.so")
        into("zygisk")
    }
}
