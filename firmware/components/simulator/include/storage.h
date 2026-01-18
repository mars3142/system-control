#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    void initialize_storage();
    void load_file(const char *filename);
    char **read_lines_filtered(const char *filename, int *out_count);
    void free_lines(char **lines, int count);
#ifdef __cplusplus
}
#endif
