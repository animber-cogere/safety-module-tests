pipeline {
    agent any
    options {
        disableConcurrentBuilds()
    }
    environment {
        GCC_DIR="C:\\gcc-arm-none-eabi\\bin"
        TOOLS_DIR="C:\\Program Files (x86)\\GnuWin32\\bin"
        MINGW64_DIR="C:\\MinGW64\\bin"
        CPPCHECK_DIR="D:\\jenkins\\tools\\CppCheck27"
        DOXYGEN_DIR="D:\\jenkins\\tools\\doxygen"
        LOCAL_TARGET_BRANCH = defineTargetBranch()
        AXIVION_TARGET_BRANCH = AxivionTargetBranch("${env.GIT_BRANCH}")
    }
    stages {
        stage('Load settings') {
            steps {
                script {
                    def properties = readProperties file: 'ci.properties'
                    env.PROJECT_NAME = properties.PROJECT_NAME
                    env.CHECKER_UUID = properties.CHECKER_UUID
                    env.COVERAGE_TARGET = properties.COVERAGE_TARGET
                }
            }
        }
    //     stage('Checkout') {
    //         steps {
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/Driver_Common']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/Driver_Common']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/main']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/googletest']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Software/Extern/googletest']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/EN61508_Program_Flow']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibQM/EN61508_Program_Flow']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/EventSystem']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/EventSystem']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/STM32_Safety_STL']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibQM/STM32_Safety_STL']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/RTOS_AL']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/RTOS_AL']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/Parameter_Table']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibQM/Parameter_Table']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/ErrorHandler']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/ErrorHandler']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/embOS']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/embOS']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/DataProcess_Averaging']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/DataProcess_Averaging']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/ErrorLogging']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/ErrorLogging']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/Devices_Temperature_TMP144']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibQM/Devices_Temperature_TMP144']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/Devices_RTC_M41T6X']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/Devices_RTC_M41T6X']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/LibCert/ST_Driver']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibCert/ST_Driver']])
    //             checkout changelog: false, poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'build/LibQM/ST_Driver']], gitTool: 'Default', userRemoteConfigs: [[credentialsId: '6737bb64-bede-48ec-ae28-3a9878078798', url: 'http://twkw-build01.twk.local/gerrit/a/TB/Controller/LibQM/ST_Driver']])
    //         }
    //     }
        stage('Build') {
            steps {
                bat """
                PATH=%PATH%;$GCC_DIR;$TOOLS_DIR
                make module
                """
                discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
                recordIssues enabledForFailure: true, publishAllIssues: true, qualityGates: [[threshold: 1, type: 'NEW', unstable: true]], tools: [gcc()]
            }
        }
    //     stage('OpenTasks') {
    //         steps {
    //             bat """
    //             PATH=%PATH%;$GCC_DIR;$TOOLS_DIR;$MINGW64_DIR;$CPPCHECK_DIR;$DOXYGEN_DIR
    //             """
    //             discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
    //             gitDiffStat()
    //             recordIssues qualityGates: [[threshold: 1, type: 'NEW', unstable: true]], tools: [taskScanner(highTags: 'FIXME,XXX', ignoreCase: true, includePattern: '**/*.c,**/*.cpp,**/*.h,**/*.hpp,**/*.vb',excludePattern: 'build/**', normalTags: 'TODO')]
    //         }
    //     }
        stage('Test') {
            steps {
                discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
                bat """
                PATH=%PATH%;$GCC_DIR;$TOOLS_DIR;$MINGW64_DIR
                make unittest
                """
                bat """
                PATH=%PATH%;$GCC_DIR;$TOOLS_DIR;$MINGW64_DIR
                unittest.exe  --gtest_output=xml:unittest.xml
                """
                xunit([GoogleTest(deleteOutputFiles: true, failIfNotNew: true, pattern: 'unittest.xml', skipNoTestFiles: false, stopProcessingIfError: true)])
            }
        }
    //     stage('CodeCoverage') {
    //         steps {
    //             catchError(buildResult: 'SUCCESS', stageResult: 'FAILURE') {
    //                 gerritCheck checks: ["${env.CHECKER_UUID}:codecoverage": 'RUNNING']
    //                 discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
    //                 bat """
    //                 PATH=%PATH%;$GCC_DIR;$TOOLS_DIR;$MINGW64_DIR
    //                 make coverage
    //                 """
    //                 recordCoverage ignoreParsingErrors: true, enabledForFailure: true,
    //                                             tools: [[parser: 'COBERTURA', pattern: '**/coverage.xml']]
    //             }
    //         }
    //     }
    //     stage('Static analysis') {
    //         steps {
    //             discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
    //             bat """
    //             PATH=%PATH%;$GCC_DIR;$TOOLS_DIR
    //             axivion\\start_analysis.bat
    //             """
    //             recordIssues enabledForFailure: true, ignoreQualityGate: false, qualityGates: [[threshold: 1, type: 'NEW', unstable: true]], tools: [axivionSuite(basedir: '', credentialsId: 'a737dbbf-9d12-44fa-a134-3cdb25c6e22e', ignoreSuppressedOrJustified: false, projectUrl: "http://twkw-build01.twk.local:9090/axivion/projects/LibCert.${env.PROJECT_NAME}.${env.AXIVION_TARGET_BRANCH}")]
    //         }
    //     }
    //     stage('Doxygen') {
    //         steps {
    //             bat """
    //             PATH=%PATH%;$GCC_DIR;$TOOLS_DIR;$MINGW64_DIR;$CPPCHECK_DIR;$DOXYGEN_DIR
    //             doxygen.cmd doc/LibCert_Module.doxyfile
    //             doc/latex/make.bat
    //             """
    //             bat """
    //             doc/latex/make.bat
    //             """
    //             bat """
    //             copy doc\\latex\\refman.pdf doc\\latex\\%PROJECT_NAME%.pdf
    //             """
    //             discoverGitReferenceBuild latestBuildIfNotFound: true, targetBranch: "${env.LOCAL_TARGET_BRANCH}"
    //             recordIssues healthy: 10, qualityGates: [[threshold: 1, type: 'NEW', unstable: true]], tools: [doxygen(pattern: '**/doxygen_error.log')]
    //             archiveArtifacts artifacts: "doc/latex/${env.PROJECT_NAME}.pdf", fingerprint: true, followSymlinks: false, onlyIfSuccessful: true
    //         }
    //     }
    //     stage('Cleanup') {
    //         steps {
    //             cleanWs()
    //         }
    //     }
    }
}
def defineTargetBranch() {
    def branchName = "${env.GERRIT_BRANCH}"
    if (branchName == 'null') {
        branchName = "${env.GIT_BRANCH}"
    }
    return branchName
}

def AxivionTargetBranch(String branch) {
    return branch.replace("/", "_")
}
