import java.io.File

plugins {
    id("com.android.application")
}

android {
    namespace = "com.zygisk.imgui"
    compileSdk = 34

    defaultConfig {
        minSdk = 24
        targetSdk = 34
        versionCode = 100
        versionName = "1.0.0"

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17 -O3 -fvisibility=hidden -fvisibility-inlines-hidden")
                arguments("-DANDROID_STL=c++_static")
                abiFilters("arm64-v8a", "armeabi-v7a", "x86_64")
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

val packageMagiskZip = tasks.register<Zip>("packageMagiskZip") {
    dependsOn("externalNativeBuildRelease")

    archiveFileName.set("Zygisk_Universal_ImGui.zip")
    destinationDirectory.set(file("${rootDir}/out"))

    from(file("${rootDir}/module.prop"))
    from(file("${rootDir}/customize.sh"))

    val nativeLibsDir = file("${layout.buildDirectory.get()}/intermediates/stripped_native_libs/release/out/lib")

    into("zygisk/arm64-v8a") {
        from(file("$nativeLibsDir/arm64-v8a/libzygisk.so"))
    }
    into("zygisk/armeabi-v7a") {
        from(file("$nativeLibsDir/armeabi-v7a/libzygisk.so"))
    }
    into("zygisk/x86_64") {
        from(file("$nativeLibsDir/x86_64/libzygisk.so"))
    }
    into("zygisk/x86") {
        from(file("$nativeLibsDir/x86/libzygisk.so"))
    }
}

tasks.named("assembleRelease") {
    finalizedBy(packageMagiskZip)
}
