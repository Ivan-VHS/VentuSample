#ifndef VEEELZEVUL_APPGUARD_H
#define VEEELZEVUL_APPGUARD_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum vee_decision { VEEELZEVUL_ALLOW=0, VEEELZEVUL_QUARANTINE=1, VEEELZEVUL_DENY=2, VEEELZEVUL_DEGRADE=3 } vee_decision;
typedef struct vee_finding {
    unsigned level_id;
    const char *rule_id;
    const char *object_id;
    vee_decision decision;
    const char *reason;
} vee_finding;
typedef void (*vee_audit_callback)(const vee_finding *finding, void *context);
typedef struct vee_guard {
    size_t max_payload_bytes, max_collection_items, max_identifier_bytes, max_path_bytes;
    vee_audit_callback audit;
    void *audit_context;
} vee_guard;
void vee_guard_init(vee_guard *guard, size_t max_payload_bytes, size_t max_collection_items,
                    size_t max_identifier_bytes, size_t max_path_bytes,
                    vee_audit_callback audit, void *audit_context);
vee_decision vee_guard_check_payload(const vee_guard *guard, unsigned level_id,
                                     const char *rule_id, const char *object_id, size_t actual_bytes);
vee_decision vee_guard_check_collection(const vee_guard *guard, unsigned level_id,
                                        const char *rule_id, const char *object_id, size_t actual_items);
vee_decision vee_guard_check_identifier(const vee_guard *guard, unsigned level_id,
                                        const char *rule_id, const char *object_id, const char *value);
vee_decision vee_guard_check_relative_path(const vee_guard *guard, unsigned level_id,
                                           const char *rule_id, const char *object_id, const char *path);
vee_decision vee_guard_authorize(const vee_guard *guard, unsigned level_id,
                                 const char *rule_id, const char *object_id, int authorized);
#ifdef __cplusplus
}
#endif
#endif
