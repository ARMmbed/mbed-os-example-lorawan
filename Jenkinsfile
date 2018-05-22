properties ([[$class: 'ParametersDefinitionProperty', parameterDefinitions: [
  [$class: 'StringParameterDefinition', name: 'mbed_os_revision', defaultValue: '', description: 'Revision of mbed-os to build. To access mbed-os PR use format "pull/PR number/head"'],
  [$class: 'BooleanParameterDefinition', name: 'regions_build_test', defaultValue: true, description: 'Test build all available regions']
  ]]])

if (env.MBED_OS_REVISION == null) {
  echo 'First run in this branch, using default parameter values'
  env.MBED_OS_REVISION = ''
}
if (env.MBED_OS_REVISION == '') {
  echo 'Using mbed OS revision from mbed-os.lib'
} else {
  echo "Using given mbed OS revision: ${env.MBED_OS_REVISION}"
  if (env.MBED_OS_REVISION.matches('pull/\\d+/head')) {
    echo "Revision is a Pull Request"
  }
}

// All available regions
def regions = [
  "\"0\"", "\"1\"", "\"2\"", "\"3\"", "\"4\"", "\"5\"", "\"6\"", "\"7\"", "\"8\"", "\"9\"",
  "\"EU868\"", "\"AS923\"", "\"AU915\"", "\"LCN470\"", "\"CN779\"", "\"EU433\"",
  "\"IN865\"", "\"KR920\"", "\"US915\"", "\"US915_HYBRID\""
]

// Supported targets
def targets = [
  "K64F"
  //"MTB_MTS_XDOT",
  //"MTB_MURATA_ABZ",
  //"MTS_MDOT_F411RE",
  //"DISCO_L072CZ_LRWAN1",
  //"MTB_ADV_WISE_1510"
]

// Map toolchains to compilers
def toolchains = [
  //ARM: "armcc",
  GCC_ARM: "arm-none-eabi-gcc"
  //IAR: "iar_arm",
  //ARMC6: "arm6"
]

def stepsForParallel = [:]

// Jenkins pipeline does not support map.each, we need to use oldschool for loop
for (int i = 0; i < targets.size(); i++) {
  for(int j = 0; j < toolchains.size(); j++) {
      def target = targets.get(i)
      def toolchain = toolchains.keySet().asList().get(j)
      def compilerLabel = toolchains.get(toolchain)

      // Skip unwanted combination
      if (target == "MTB_MURATA_ABZ" && toolchain == "GCC_ARM") {
        continue
      }
      if (target == "DISCO_L072CZ_LRWAN1" && toolchain == "GCC_ARM") {
        continue
      }

      def stepName = "${target} ${toolchain}"

      stepsForParallel[stepName] = buildStep(target, compilerLabel, toolchain)
  }
}

def stepsForRegional = [:]

if (params.regions_build_test == true) {
  stepsForRegional["REGION BUILDER"] = build_regions(regions)
}

timestamps {
  parallel stepsForParallel
  parallel stepsForRegional
}

def buildStep(target, compilerLabel, toolchain) {
  return {
    stage ("${target}_${compilerLabel}") {
      node ("${compilerLabel}") {
        deleteDir()
        dir("mbed-os-example-lorawan") {
          checkout scm

          if (isUnix()) {
            sh "mbed deploy --protocol ssh"
          } else {
            bat "mbed deploy --protocol ssh"
          }
          // Set mbed-os to revision received as parameter
          if (env.MBED_OS_REVISION != '') {
            dir("mbed-os") {
              if (env.MBED_OS_REVISION.matches('pull/\\d+/head')) {
                // Use mbed-os PR and switch to branch created
                if (isUnix()) {
                  sh "git fetch origin ${env.MBED_OS_REVISION}:_PR_"
                  sh "git checkout _PR_"
                } else {
                  bat "git fetch origin ${env.MBED_OS_REVISION}:_PR_"
                  bat "git checkout _PR_"
                }

              } else {
                if (isUnix()) {
                  sh "git checkout ${env.MBED_OS_REVISION}"
                } else {
                  bat "git checkout ${env.MBED_OS_REVISION}"
                }
              }
            }
          }

          // Adjust stack size and crystal values
          if ("${target}" == "DISCO_L072CZ_LRWAN1") {
            if (isUnix()) {
              sh "sed -i 's/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x10)/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x13)/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/device/stm32l0xx_hal_rcc.h"
            } else {
              bat "sed -i 's/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x10)/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x13)/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/device/stm32l0xx_hal_rcc.h"
            } 
          }

          if ("${target}" == "MTB_MURATA_ABZ") {
            if (isUnix()) {
              sh "sed -i 's/define symbol __size_heap__   = 0x800;/define symbol __size_heap__   = 0x1000;/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/TARGET_STM32L0x2xZ/device/TOOLCHAIN_IAR/stm32l082xZ.icf"
              sh "sed -i 's/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x10)/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x16)/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/device/stm32l0xx_hal_rcc.h"
            } else {
              bat "sed -i 's/define symbol __size_heap__   = 0x800;/define symbol __size_heap__   = 0x1000;/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/TARGET_STM32L0x2xZ/device/TOOLCHAIN_IAR/stm32l082xZ.icf"
              bat "sed -i 's/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x10)/#define RCC_HSICALIBRATION_DEFAULT       ((uint32_t)0x16)/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L0/device/stm32l0xx_hal_rcc.h"
            } 
          }

          if ("${target}" == "MTB_MTS_XDOT") {
            if (isUnix()) {
              sh "sed -i 's/define symbol __size_heap__   = 0x800;/define symbol __size_heap__   = 0x1800;/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L1/TARGET_MTB_MTS_XDOT/device/TOOLCHAIN_IAR/stm32l152xc.icf"
            } else {
              bat "sed -i 's/define symbol __size_heap__   = 0x800;/define symbol __size_heap__   = 0x1800;/' \
              mbed-os/targets/TARGET_STM/TARGET_STM32L1/TARGET_MTB_MTS_XDOT/device/TOOLCHAIN_IAR/stm32l152xc.icf"
            } 
          }

          if (isUnix()) {
            sh "mbed compile --build out/${target}_${toolchain}/ -m ${target} -t ${toolchain} -c"
          } else {
            bat "mbed compile --build out/${target}_${toolchain}/ -m ${target} -t ${toolchain} -c"
          }
        }
        stash name: "${target}_${toolchain}", includes: '**/mbed-os-example-lorawan.bin'
        archive '**/mbed-os-example-lorawan.bin'
        step([$class: 'WsCleanup'])
      }
    }
  }
}

def build_regions(regions)
  return {
    stage ("region_builder_K64F_GCC_ARM") {
      node ("arm-none-eabi-gcc") {
        deleteDir()
        dir("mbed-os-example-lorawan") {
          checkout scm
          //sh "mbed deploy --protocol ssh"
          //mbed-os.lib??
          //Initial sed to get list replacing working
          sh "sed -i 's/\"lora.phy\": 0,/\"lora.phy\": \"0\",/' mbed_app.json"
          //build 0 EU tested above already
          for (int i = 1; i < regions.size(); i++) {
            def curr_region = regions.get(i)
            def prev_region = regions.get(i-1)
            echo "Current: ${curr_region}, previous: ${prev_region}"
            sh "sed -i 's/\"lora.phy\": ${prev_region},/\"lora.phy\": ${curr_region},/' mbed_app.json"
            sh "cat mbed_app.json"
            //sh "mbed compile -t GCC_ARM -m K64F"
          }
        }
      }
    }
  }
}
