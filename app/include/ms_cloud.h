#ifndef _MS_CLOUD_H_
#define _MS_CLOUD_H_

void init_cloud(void);
void publish(const char *event, const char *json_fmt, ...);
void shadow_accept_timeout_cb(void *arg);
#endif
