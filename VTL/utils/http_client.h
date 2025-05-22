#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

int http_get(const char *url);
int http_post(const char *url, const char *payload);
int http_post_with_headers(const char *url, const char *payload, const char **headers, int headers_count);
int http_post_multipart(const char *url, const char *field_name, const char *file_path, const char *chat_id, const char *caption);
int http_post_multipart_group(const char *url, const char *chat_id, const char *json_metadata, int count, char **file_paths);
int http_get_with_headers(const char *url, const char **headers, int headers_count);

#endif
