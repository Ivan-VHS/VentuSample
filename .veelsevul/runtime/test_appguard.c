#include "appguard.h"
#include <assert.h>
#include <stdio.h>
typedef struct audit_state { unsigned count, denied, degraded; } audit_state;
static void capture_audit(const vee_finding *f, void *context) {
    audit_state *s=(audit_state *)context;
    assert(f!=NULL && f->level_id>=1U && f->level_id<=666U);
    assert(f->rule_id!=NULL && f->rule_id[0]!='\0');
    assert(f->object_id!=NULL && f->object_id[0]!='\0');
    assert(f->reason!=NULL && f->reason[0]!='\0');
    s->count++;
    if (f->decision==VEEELZEVUL_DENY) s->denied++;
    if (f->decision==VEEELZEVUL_DEGRADE) s->degraded++;
}
int main(void) {
    vee_guard g;
    audit_state a={0U,0U,0U};
    vee_guard_init(&g,1024U,32U,64U,128U,capture_audit,&a);
    assert(vee_guard_check_payload(&g,112U,"A-09","request-1",512U)==VEEELZEVUL_ALLOW);
    assert(vee_guard_check_payload(&g,112U,"A-09","request-2",2048U)==VEEELZEVUL_DENY);
    assert(vee_guard_check_collection(&g,334U,"M-13","batch-1",16U)==VEEELZEVUL_ALLOW);
    assert(vee_guard_check_collection(&g,334U,"M-13","batch-2",64U)==VEEELZEVUL_DEGRADE);
    assert(vee_guard_check_identifier(&g,223U,"M-03","plugin-1","plugin-v1")==VEEELZEVUL_ALLOW);
    assert(vee_guard_check_identifier(&g,223U,"M-03","plugin-2","bad/name")==VEEELZEVUL_DENY);
    assert(vee_guard_check_relative_path(&g,445U,"A-12","plugin-path-1","plugins/eq.vst3")==VEEELZEVUL_ALLOW);
    assert(vee_guard_check_relative_path(&g,445U,"A-12","plugin-path-2","../outside.dll")==VEEELZEVUL_DENY);
    assert(vee_guard_check_relative_path(&g,445U,"A-12","plugin-path-3","/etc/passwd")==VEEELZEVUL_DENY);
    assert(vee_guard_authorize(&g,223U,"Я-03","driver-install",1)==VEEELZEVUL_ALLOW);
    assert(vee_guard_authorize(&g,223U,"Я-03","driver-install",0)==VEEELZEVUL_DENY);
    assert(vee_guard_authorize(&g,223U,"Я-03","driver-install",-1)==VEEELZEVUL_DENY);
    assert(a.count==12U && a.denied>=5U && a.degraded==1U);
    puts("Veelsevul AppGuard runtime smoke tests: PASS");
    return 0;
}
