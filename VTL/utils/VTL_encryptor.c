#include <VTL/utils/VTL_encryptor.h>
// #include <openssl/evp.h>
#include <string.h>
#include <stdio.h>

const char* SECRET_KEY = "mysecretkey12345678901234567890";

void VTL_Encrypt(const VTL_StandartString src_string, VTL_EncryptedString encrypted_string) {
    encrypted_string = src_string;
    // Uncomment the following lines if you want to use OpenSSL for encryption
    // EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    // if (!ctx) {
    //     fprintf(stderr, "Failed to create EVP_CIPHER_CTX\n");
    //     return;
    // }
    //
    // const EVP_CIPHER *cipher = EVP_aes_256_ecb(); // Use ECB mode (no IV)
    // if (EVP_EncryptInit_ex(ctx, cipher, NULL, (unsigned char *)SECRET_KEY, NULL) != 1) {
    //     fprintf(stderr, "Failed to initialize encryption\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // int src_len = strlen((const char *)src_string);
    // int encrypted_len = 0;
    // int final_len = 0;
    //
    // if (EVP_EncryptUpdate(ctx, encrypted_string, &encrypted_len, (unsigned char *)src_string, src_len) != 1) {
    //     fprintf(stderr, "Failed to encrypt data\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // if (EVP_EncryptFinal_ex(ctx, encrypted_string + encrypted_len, &final_len) != 1) {
    //     fprintf(stderr, "Failed to finalize encryption\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // encrypted_len += final_len;
    // encrypted_string[encrypted_len] = '\0'; // Null-terminate the encrypted string
    //
    // EVP_CIPHER_CTX_free(ctx);
}

void VTL_Decrypt(const VTL_EncryptedString encrypted_string, VTL_StandartString decrypted_string) {
    decrypted_string = encrypted_string;
    // EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    // if (!ctx) {
    //     fprintf(stderr, "Failed to create EVP_CIPHER_CTX\n");
    //     return;
    // }
    //
    // const EVP_CIPHER *cipher = EVP_aes_256_ecb(); // Use ECB mode (no IV)
    // if (EVP_DecryptInit_ex(ctx, cipher, NULL, (unsigned char *)SECRET_KEY, NULL) != 1) {
    //     fprintf(stderr, "Failed to initialize decryption\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // int encrypted_len = strlen((const char *)encrypted_string);
    // int decrypted_len = 0;
    // int final_len = 0;
    //
    // if (EVP_DecryptUpdate(ctx, decrypted_string, &decrypted_len, (unsigned char *)encrypted_string, encrypted_len) != 1) {
    //     fprintf(stderr, "Failed to decrypt data\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // if (EVP_DecryptFinal_ex(ctx, decrypted_string + decrypted_len, &final_len) != 1) {
    //     fprintf(stderr, "Failed to finalize decryption\n");
    //     EVP_CIPHER_CTX_free(ctx);
    //     return;
    // }
    //
    // decrypted_len += final_len;
    // decrypted_string[decrypted_len] = '\0'; // Null-terminate the decrypted string
    //
    // EVP_CIPHER_CTX_free(ctx);
}