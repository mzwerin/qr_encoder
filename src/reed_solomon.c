#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to call the Python script and fetch Reed-Solomon encoded data
void call_reedsolomon(const char *hex_string, int error_level, char *output, size_t output_size) {
    char command[8192];

    // Construct the Python command
    snprintf(command, sizeof(command),
             "python3 -c \"import reedsolo"
             "data = bytes.fromhex('%s')"
             "rs = reedsolo.RSCodec(%d)"
             "encoded_data = rs.encode(data)"
             "parity = encoded_data[-%d:]"
             "print(parity.hex().upper())\"",
             hex_string, error_level, error_level);

    // Execute the command and capture the output
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run Python command");
        exit(1);
    }

    // Read the output from Python script
    if (fgets(output, output_size, fp) == NULL) {
        perror("Failed to read Python output");
        exit(1);
    }
    output[strcspn(output, "\n")] = 0;

    pclose(fp);
}

/*
// Main function to test the Python call
int main() {
    char input[256];
    char encoded_data[512];
    int error_level;

    // Prompt user for input string
    printf("Enter input hex string: ");
    fgets(input, sizeof(input), stdin);

    // Remove trailing newline character if present
    input[strcspn(input, "\n")] = '\0';

    // Prompt user for error level
    printf("Enter error level (number of bytes): ");
    scanf("%d", &error_level);

    // Call Python function to get Reed-Solomon encoded data
    call_reedsolomon(input, error_level, encoded_data, sizeof(encoded_data));

    // Print the output
    printf("Input: %s\n", input);
    printf("Reed-Solomon Parity Bytes: %s\n", encoded_data);

    return 0;
}
*/