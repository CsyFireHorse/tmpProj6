#ifndef _ENI_JSON_SLICE_H_
#define _ENI_JSON_SLICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Locate embedded JSON in a null-terminated ENI content buffer. */
int extract_eni_json_slice(const char* eni_content, int* ofs, int* length);

#ifdef __cplusplus
}
#endif

#endif
