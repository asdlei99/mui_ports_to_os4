/* support functions */

#ifndef CHCASTLE_SUPPORT_H
#define CHCASTLE_SUPPORT_H

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include <stdint.h>
#include <stdio.h>

#include "methodlist.h"
#include "generator.h"

#define UNUSED __attribute__((unused))

#define CHC_VERSION "0.8"
#define CHC_DATE    "22.08.2016"

#define PROJECT_TYPE_MUI        1
#define PROJECT_TYPE_REGGAE     2

//extern struct Library *UtilityBase, *SysBase, *DOSBase, *MUIMasterBase, *IntuitionBase;

/* Common MUI stuff */

#define xset(obj, attr, val) SetAttrs(obj, attr, (IPTR)val, TAG_END)
#define findobj(parent, id) (Object*)DoMethod(parent, MUIM_FindUData, id)



static inline intptr_t xget(Object *obj, uint32_t attr)
{
#ifdef __amigaos4__
	uint32 v = 0;
#else
	uintptr_t v = 0;
#endif

	GetAttr(attr, obj, &v);
	return (intptr_t)v;
}


/* Filter tables for string gadgets, and tables for popups */

extern STRPTR CTypedef;
extern STRPTR M68kRegs[];
extern const STRPTR IdAllowed;
extern const STRPTR HexAllowed;
extern const struct MethodEntry ReggaeDefMethods[];

extern const char ScreenTitle[];

extern APTR MPool;
extern Object *App, *GenWin, *InWin;

#define T(x) DoMethod(obj, GENM_Text, (IPTR)x)
#define TC(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_CLASSNAME)
#define TCS(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_CLASSNAMELC)
#define TLN(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_LIBNAME)
#define TVER(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_VERSION)
#define TREV(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_REVISION)
#define TCPR(x) DoMethod(obj, GENM_TextParam, (IPTR)x, PARAM_COPYRIGHT)
#define LEAD(x) DoMethod(obj, GENM_LeadingComment, (IPTR)x)
#define TRAIL(x) DoMethod(obj, GENM_TrailingComment, (IPTR)x)

#define I DoMethod(obj, GENM_DoIndent);
#define II DoMethod(obj, GENM_IndentIn);
#define IO DoMethod(obj, GENM_IndentOut);
#define TODO(x) DoMethod(obj, GENM_InsertRemark, (IPTR)x)

#define SUBWINDOW_CLOSE 0x6EDA888A

void strlow(STRPTR string);
void error(STRPTR msg, ...);

void generate_emullibentry(Object *obj);
void generate_dispatcher(Object *obj, Object *method_list);
void wait_loop(void);
Object *labelled_checkmark(STRPTR label, IPTR id);

#define MAXPARAMS 8
#define MUIV_Application_ReturnID_RemGen   0x6EDA9938

struct Parser
{
	int32_t params[MAXPARAMS];
	struct RDArgs *args;
	int line;
};

BOOL check_pattern(STRPTR line, STRPTR pattern, struct Parser *parser);

void unexpected_eof(Object *obj, struct Parser *p);
void syntax_error(Object *obj, struct Parser *p);


Object* MUI_NewObjectM(char *classname, ...);
Object* DoSuperNewM(Class *cl, Object *obj, ...);
Object* NewObjectM(Class *cl, char *classname, ...);

/*############################################################################*/
/*############################################################################*/

inline extern IPTR strln(STRPTR s)
{
	STRPTR v = s;

	while (*v) v++;
	return (IPTR)(v - s);
}

/*############################################################################*/
/*############################################################################*/

inline extern void newlist(struct MinList *list)
{
	list->mlh_Head = (struct MinNode*)&list->mlh_Tail;
	list->mlh_Tail = NULL;
	list->mlh_TailPred = (struct MinNode*)&list->mlh_Head;
}

/*############################################################################*/
/*############################################################################*/

#endif  /* CHCASTLE_SUPPORT_H */