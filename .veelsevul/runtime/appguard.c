#include "appguard.h"
#include <string.h>
static int vee_valid_context(unsigned level, const char *rule, const char *object) {
    return level >= 1U && level <= 666U && rule != NULL && rule[0] != '\0' &&
           object != NULL && object[0] != '\0';
}
static vee_decision vee_emit(const vee_guard *guard, unsigned level, const char *rule,
                             const char *object, vee_decision decision, const char *reason) {
    if (guard != NULL && guard->audit != NULL && vee_valid_context(level, rule, object)) {
        const vee_finding finding = {level, rule, object, decision, reason};
        guard->audit(&finding, guard->audit_context);
    }
    return decision;
}
void vee_guard_init(vee_guard *guard, size_t payload, size_t items, size_t identifier,
                    size_t path, vee_audit_callback audit, void *context) {
    if (guard == NULL) return;
    guard->max_payload_bytes = payload != 0U ? payload : 1024U * 1024U;
    guard->max_collection_items = items != 0U ? items : 4096U;
    guard->max_identifier_bytes = identifier != 0U ? identifier : 128U;
    guard->max_path_bytes = path != 0U ? path : 1024U;
    guard->audit = audit;
    guard->audit_context = context;
}
vee_decision vee_guard_check_payload(const vee_guard *g, unsigned l, const char *r,
                                     const char *o, size_t actual) {
    if (g == NULL || !vee_valid_context(l,r,o)) return VEEELZEVUL_DENY;
    if (actual > g->max_payload_bytes)
        return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"payload byte ceiling exceeded");
    return vee_emit(g,l,r,o,VEEELZEVUL_ALLOW,"payload byte ceiling satisfied");
}
vee_decision vee_guard_check_collection(const vee_guard *g, unsigned l, const char *r,
                                        const char *o, size_t actual) {
    if (g == NULL || !vee_valid_context(l,r,o)) return VEEELZEVUL_DENY;
    if (actual > g->max_collection_items)
        return vee_emit(g,l,r,o,VEEELZEVUL_DEGRADE,"collection item ceiling exceeded");
    return vee_emit(g,l,r,o,VEEELZEVUL_ALLOW,"collection item ceiling satisfied");
}
vee_decision vee_guard_check_identifier(const vee_guard *g, unsigned l, const char *r,
                                        const char *o, const char *value) {
    size_t i, n;
    if (g == NULL || !vee_valid_context(l,r,o) || value == NULL) return VEEELZEVUL_DENY;
    n = strlen(value);
    if (n == 0U || n > g->max_identifier_bytes || value[0] == '.')
        return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"identifier length or prefix rejected");
    for (i=0U; i<n; ++i) {
        const unsigned char c=(unsigned char)value[i];
        const int ok=(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||
                     c=='_'||c=='-'||c=='.'||c==':';
        if (!ok) return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"identifier contains a disallowed byte");
    }
    return vee_emit(g,l,r,o,VEEELZEVUL_ALLOW,"identifier allowlist satisfied");
}
vee_decision vee_guard_check_relative_path(const vee_guard *g, unsigned l, const char *r,
                                           const char *o, const char *path) {
    const char *segment, *cursor;
    size_t n;
    if (g==NULL || !vee_valid_context(l,r,o) || path==NULL || path[0]=='\0') return VEEELZEVUL_DENY;
    n=strlen(path);
    if (n>g->max_path_bytes || path[0]=='/' || path[0]=='\\' || strchr(path,'\\')!=NULL ||
        strchr(path,':')!=NULL || path[n-1U]=='/')
        return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"path is absolute, malformed or over limit");
    segment=path;
    for (cursor=path;;++cursor) {
        const unsigned char c=(unsigned char)*cursor;
        if (c!=0U && (c<0x20U || c==0x7fU))
            return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"path contains a control byte");
        if (c=='/' || c==0U) {
            const size_t len=(size_t)(cursor-segment);
            if (len==0U || (len==1U&&segment[0]=='.') ||
                (len==2U&&segment[0]=='.'&&segment[1]=='.'))
                return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"path traversal or empty segment rejected");
            if (c==0U) break;
            segment=cursor+1;
        }
    }
    return vee_emit(g,l,r,o,VEEELZEVUL_ALLOW,"relative path boundary satisfied");
}
vee_decision vee_guard_authorize(const vee_guard *g, unsigned l, const char *r,
                                 const char *o, int authorized) {
    if (g==NULL || !vee_valid_context(l,r,o)) return VEEELZEVUL_DENY;
    if (authorized!=1) return vee_emit(g,l,r,o,VEEELZEVUL_DENY,"explicit authorization was not granted");
    return vee_emit(g,l,r,o,VEEELZEVUL_ALLOW,"explicit authorization granted");
}
