plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.touchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34
        
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }
}

tasks.register("buildMagiskModule", Zip::class) {
    dependsOn("assembleRelease")
    archiveFileName.set("zygisk-touch-fix-module.zip")
    destinationDirectory.set(file("$buildDir/outputs/magisk"))

    from("src/main/magisk") {
        include("module.prop", "customize.sh", "post-fs-data.sh", "service.sh")
    }
    
    // Package compiled native library into zygisk folder structure
    from("$buildDir/intermediates/cmake/release/obj") {
        into("zygisk")
    }
}
