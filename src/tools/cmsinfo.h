#if !defined(_CMSINFO_H_INCLUDED_)
#define _CMSINFO_H_INCLUDED_

#define COMMAND_GET_URL_PATTERN         1
#define COMMAND_CHECK_URL_PATTERN       2
#define COMMAND_CHECK_HTML_PATTERN      3
#define COMMAND_GET_HTML_PATTERN        4

#define S_HEAD "20090518"
#ifdef  __cplusplus
extern "C" {
#endif
int fnCmsinfo(void);
char *CheckUrlPattern(char *url, char *pattern, char *out);
char *CheckHtmlPattern(char *url, char *tmpl);
char *GetUrlsPattern(char *url);
char *GetHtmlPattern (char *url, char *title, char *pubdate, 
                          char *src, char *writer, char *content, char *checkurl);
void SetServer(char *_ip, int _port);
int GetResult(int len, char *content, int command, char **out);
#ifdef  __cplusplus
}
#endif

#endif
