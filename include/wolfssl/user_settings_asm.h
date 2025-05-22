#pragma once
#ifdef __cplusplus
#endif
#define WOLF_CONF_DEBUG      0
#define WOLF_CONF_WOLFCRYPT_ONLY      0
#define WOLF_CONF_TLS13      1
#define WOLF_CONF_TLS12      1
#define WOLF_CONF_DTLS      0
#define WOLF_CONF_MATH      1
#define WOLF_CONF_RTOS      1
#define WOLF_CONF_RNG      1
#define WOLF_CONF_RSA      1
#define WOLF_CONF_ECC      1
#define WOLF_CONF_DH      1
#define WOLF_CONF_AESGCM      1
#define WOLF_CONF_AESCBC      0
#define WOLF_CONF_CHAPOLY      1
#define WOLF_CONF_EDCURVE25519      0
#define WOLF_CONF_MD5      0
#define WOLF_CONF_SHA1      0
#define WOLF_CONF_SHA2_224      0
#define WOLF_CONF_SHA2_256      1
#define WOLF_CONF_SHA2_384      0
#define WOLF_CONF_SHA2_512      1
#define WOLF_CONF_SHA3      0
#define WOLF_CONF_PSK      0
#define WOLF_CONF_PWDBASED      0
#define WOLF_CONF_KEEP_PEER_CERT      0
#define WOLF_CONF_BASE64_ENCODE      0
#define WOLF_CONF_OPENSSL_EXTRA      0
#define WOLF_CONF_TEST      1
#define WOLF_CONF_PQM4      0
#define NO_STM32_HASH
#define NO_STM32_CRYPTO
#if defined(STM32WB55xx)
    #define WOLFSSL_STM32WB
    #define WOLFSSL_STM32_PKA
    #undef  NO_STM32_CRYPTO
    #define HAL_CONSOLE_UART huart1
#elif defined(STM32F407xx)
    #define WOLFSSL_STM32F4
    #define HAL_CONSOLE_UART huart2
#elif defined(STM32F437xx)
    #define WOLFSSL_STM32F4
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define STM32_HAL_V2
    #define HAL_CONSOLE_UART huart4
#elif defined(STM32F777xx)
    #define WOLFSSL_STM32F7
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define STM32_HAL_V2
    #define HAL_CONSOLE_UART huart2
    #define STM32_AESGCM_PARTIAL /* allow partial blocks and add auth info (header) */
#elif defined(STM32H735xx)
    #define WOLFSSL_STM32H7
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define HAL_CONSOLE_UART huart5
#elif defined(STM32H753xx)
    #define WOLFSSL_STM32H7
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define HAL_CONSOLE_UART huart3
#elif defined(STM32L4A6xx)
    #define WOLFSSL_STM32L4
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define HAL_CONSOLE_UART hlpuart1
#elif defined(STM32L475xx)
    #define WOLFSSL_STM32L4
    #define HAL_CONSOLE_UART huart1
#elif defined(STM32L562xx)
    #define WOLFSSL_STM32L5
    #define WOLFSSL_STM32_PKA
    #undef  NO_STM32_HASH
    #undef  NO_STM32_CRYPTO
    #define HAL_CONSOLE_UART huart1
#elif defined(STM32L552xx)
    #define WOLFSSL_STM32L5
    #undef  NO_STM32_HASH
    #define HAL_CONSOLE_UART hlpuart1
#elif defined(STM32F207xx)
    #define WOLFSSL_STM32F2
    #define HAL_CONSOLE_UART huart3
#elif defined(STM32F107xC)
    #define WOLFSSL_STM32F1
    #define HAL_CONSOLE_UART huart4
    #define NO_STM32_RNG
#elif defined(STM32F401xE)
    #define WOLFSSL_STM32F4
    #define HAL_CONSOLE_UART huart2
    #define NO_STM32_RNG
    #define WOLFSSL_GENSEED_FORTEST
#elif defined(STM32G071xx)
    #define WOLFSSL_STM32G0
    #define HAL_CONSOLE_UART huart2
    #define NO_STM32_RNG
    #define WOLFSSL_GENSEED_FORTEST
#elif defined(STM32U575xx)
    #define HAL_CONSOLE_UART huart1
    #define WOLFSSL_STM32U5
    #define STM32_HAL_V2
#else
    #warning Please define a hardware platform!
    #define WOLFSSL_STM32F4
    #define HAL_CONSOLE_UART huart4
#endif
#define SIZEOF_LONG_LONG 8
#define WOLFSSL_GENERAL_ALIGNMENT 4
#define WOLFSSL_STM32_CUBEMX
#define WOLFSSL_SMALL_STACK
#define WOLFSSL_USER_IO
#define WOLFSSL_NO_SOCK
#define WOLFSSL_CERT_GEN
#if defined(WOLF_CONF_RTOS) && WOLF_CONF_RTOS == 2
    #define FREERTOS
#else
    #define SINGLE_THREADED
#endif
#if defined(WOLF_CONF_MATH) && WOLF_CONF_MATH != 2
    #define USE_FAST_MATH
    #define TFM_TIMING_RESISTANT
#endif
#if defined(WOLF_CONF_MATH) && (WOLF_CONF_MATH >= 3)
    #define WOLFSSL_SP
    #if WOLF_CONF_MATH != 7
        #define WOLFSSL_SP_SMALL      /* use smaller version of code */
    #endif
    #if defined(WOLF_CONF_RSA) && WOLF_CONF_RSA == 1
        #define WOLFSSL_HAVE_SP_RSA
    #endif
    #if defined(WOLF_CONF_DH) && WOLF_CONF_DH == 1
        #define WOLFSSL_HAVE_SP_DH
    #endif
    #if defined(WOLF_CONF_ECC) && WOLF_CONF_ECC == 1
        #define WOLFSSL_HAVE_SP_ECC
    #endif
    #if WOLF_CONF_MATH == 6 || WOLF_CONF_MATH == 7
        #define WOLFSSL_SP_MATH    /* disable non-standard curves / key sizes */
    #endif
    #define SP_WORD_SIZE 32
    #if WOLF_CONF_MATH == 4 || WOLF_CONF_MATH == 5
        #define WOLFSSL_SP_ASM /* required if using the ASM versions */
        #if WOLF_CONF_MATH == 4
            #define WOLFSSL_SP_ARM_CORTEX_M_ASM
        #endif
        #if WOLF_CONF_MATH == 5
            #define WOLFSSL_SP_ARM_THUMB_ASM
        #endif
    #endif
#endif
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_ENCRYPT_THEN_MAC
#define HAVE_EXTENDED_MASTER
#if defined(WOLF_CONF_TLS13) && WOLF_CONF_TLS13 == 1
    #define WOLFSSL_TLS13
    #define HAVE_HKDF
#endif
#if defined(WOLF_CONF_DTLS) && WOLF_CONF_DTLS == 1
    #define WOLFSSL_DTLS
#endif
#if defined(WOLF_CONF_PSK) && WOLF_CONF_PSK == 0
    #define NO_PSK
#endif
#if defined(WOLF_CONF_PWDBASED) && WOLF_CONF_PWDBASED == 0
    #define NO_PWDBASED
#endif
#if defined(WOLF_CONF_KEEP_PEER_CERT) && WOLF_CONF_KEEP_PEER_CERT == 1
    #define KEEP_PEER_CERT
#endif
#if defined(WOLF_CONF_BASE64_ENCODE) && WOLF_CONF_BASE64_ENCODE == 1
    #define WOLFSSL_BASE64_ENCODE
#endif
#if defined(WOLF_CONF_OPENSSL_EXTRA) && WOLF_CONF_OPENSSL_EXTRA == 1
    #define OPENSSL_EXTRA
#endif
#if 0
    #define SMALL_SESSION_CACHE
#else
    #define NO_SESSION_CACHE
#endif
#undef NO_RSA
#if defined(WOLF_CONF_RSA) && WOLF_CONF_RSA == 1
    #ifdef USE_FAST_MATH
        #undef  FP_MAX_BITS
        #define FP_MAX_BITS     4096
    #endif
    #undef  RSA_LOW_MEM
    #undef  WC_RSA_BLINDING
    #define WC_RSA_BLINDING
    #ifdef WOLFSSL_TLS13
        #define WC_RSA_PSS
    #endif
#else
    #define NO_RSA
#endif
#undef HAVE_ECC
#if defined(WOLF_CONF_ECC) && WOLF_CONF_ECC == 1
    #define HAVE_ECC
    #define ECC_USER_CURVES
    #undef NO_ECC256
    #undef  FP_ECC
    #ifdef FP_ECC
        #undef  FP_ENTRIES
        #define FP_ENTRIES  2
        #undef  FP_LUT
        #define FP_LUT      4
    #endif
    #undef  ECC_SHAMIR
    #define ECC_SHAMIR
    #define ECC_TIMING_RESISTANT
    #ifdef USE_FAST_MATH
        #ifdef NO_RSA
            #define FP_MAX_BITS     (256 * 2)
        #else
            #define ALT_ECC_SIZE
        #endif
    #endif
#endif
#undef NO_DH
#if defined(WOLF_CONF_DH) && WOLF_CONF_DH == 1
    #define HAVE_DH /* freeRTOS settings.h requires this */
    #define HAVE_FFDHE_2048
    #define HAVE_DH_DEFAULT_PARAMS
#else
    #define NO_DH
#endif
#if defined(WOLF_CONF_AESGCM) && WOLF_CONF_AESGCM == 1
    #define HAVE_AESGCM
    #define GCM_SMALL
    #define HAVE_AES_DECRYPT
#endif
#if defined(WOLF_CONF_AESCBC) && WOLF_CONF_AESCBC == 1
    #define HAVE_AES_CBC
    #define HAVE_AES_DECRYPT
#endif
#undef HAVE_CHACHA
#undef HAVE_POLY1305
#if defined(WOLF_CONF_CHAPOLY) && WOLF_CONF_CHAPOLY == 1
    #define HAVE_CHACHA
    #define HAVE_POLY1305
    #undef  HAVE_ONE_TIME_AUTH
    #define HAVE_ONE_TIME_AUTH
#endif
#undef HAVE_CURVE25519
#undef HAVE_ED25519
#if defined(WOLF_CONF_EDCURVE25519) && WOLF_CONF_EDCURVE25519 == 1
    #define HAVE_CURVE25519
    #define HAVE_ED25519
    #define CURVED25519_SMALL
#endif
#undef NO_SHA
#if defined(WOLF_CONF_SHA1) && WOLF_CONF_SHA1 == 1
#else
    #define NO_SHA
#endif
#undef NO_SHA256
#if defined(WOLF_CONF_SHA2_256) && WOLF_CONF_SHA2_256 == 1
    #if defined(WOLF_CONF_SHA2_224) && WOLF_CONF_SHA2_224 == 1
        #define WOLFSSL_SHA224
    #endif
#else
    #define NO_SHA256
#endif
#undef WOLFSSL_SHA512
#if defined(WOLF_CONF_SHA2_512) && WOLF_CONF_SHA2_512 == 1
    #define WOLFSSL_SHA512
    #define HAVE_SHA512 /* freeRTOS settings.h requires this */
#endif
#undef WOLFSSL_SHA384
#if defined(WOLF_CONF_SHA2_384) && WOLF_CONF_SHA2_384 == 1
    #define WOLFSSL_SHA384
#endif
#undef WOLFSSL_SHA3
#if defined(WOLF_CONF_SHA3) && WOLF_CONF_SHA3 == 1
#endif
#if defined(WOLF_CONF_MD5) && WOLF_CONF_MD5 == 1
#else
    #define NO_MD5
#endif
#define BENCH_EMBEDDED
#define USE_CERT_BUFFERS_2048
#define USE_CERT_BUFFERS_256
#if defined(WOLF_CONF_DEBUG) && WOLF_CONF_DEBUG == 1
    #define DEBUG_WOLFSSL
    #if 0
        #define USE_WOLFSSL_MEMORY
        #define WOLFSSL_TRACK_MEMORY
    #endif
#else
#endif
#define WOLFSSL_USER_CURRTIME
#define NO_OLD_RNGNAME /* conflicts with STM RNG macro */
#if !defined(WOLF_CONF_RNG) || WOLF_CONF_RNG == 1
    #define HAVE_HASHDRBG
#else /* WOLF_CONF_RNG == 0 */
    #define WC_NO_HASHDRBG
    #define WC_NO_RNG
#endif
#if defined(WOLF_CONF_TLS12) && WOLF_CONF_TLS12 == 0
    #define WOLFSSL_NO_TLS12
#endif
#if defined(WOLF_CONF_WOLFCRYPT_ONLY) && WOLF_CONF_WOLFCRYPT_ONLY == 1
    #define WOLFCRYPT_ONLY
#endif
#if defined(WOLF_CONF_TEST) && WOLF_CONF_TEST == 0
    #define NO_CRYPT_TEST
    #define NO_CRYPT_BENCHMARK
#endif
#define NO_FILESYSTEM
#define NO_WRITEV
#ifndef NO_MAIN_DRIVER
#define NO_MAIN_DRIVER
#endif
#define NO_DEV_RANDOM
#define NO_OLD_TLS
#define WOLFSSL_NO_CLIENT_AUTH /* disable client auth for Ed25519/Ed448 */
#define NO_DSA
#define NO_RC4
#define NO_MD4
#define NO_DES3
#define WOLFSSL_GMTIME
#define WOLFSSH_SCP
#define WOLFSSH_SFTP
#define WOLFSSH_USER_FILESYSTEM
#define WOLFSSH_STOREHANDLE
#ifndef __ASSEMBLY__
#include "wolffs.h"
#endif
#ifdef __cplusplus
#endif
