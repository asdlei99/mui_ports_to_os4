/* support functions */

#include "support.h"
#include "generator.h"
#include "locale.h"
#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#endif
#include <libvstring.h>
#include <SDI_stdarg.h>

/* Filter tables for string gadgets */

const char ScreenTitle[] = "Chocolate Castle " CHC_VERSION;

const STRPTR IdAllowed = (const STRPTR)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
const STRPTR HexAllowed = (const STRPTR)"ABCDEFabcdef0123456789";

/* LOW LEVEL TEXT/MUI FUNCTIONS */

/// table of default methods for a Reggae class.

const struct MethodEntry ReggaeDefMethods[] = {
	{ (STRPTR)"OM_NEW", (STRPTR)"New", (STRPTR)"opSet", (STRPTR)"00000101", TRUE },
	{ (STRPTR)"OM_DISPOSE", (STRPTR)"Dispose", (STRPTR)"", (STRPTR)"00000102", TRUE },
	{ (STRPTR)"OM_SET", (STRPTR)"Set", (STRPTR)"opSet", (STRPTR)"00000103", TRUE },
	{ (STRPTR)"OM_GET", (STRPTR)"Get", (STRPTR)"opGet", (STRPTR)"00000104", TRUE },
	{ (STRPTR)"MMM_Setup", (STRPTR)"Setup", (STRPTR)"mmopSetup", (STRPTR)"8EDA0050", TRUE },
	{ (STRPTR)"MMM_Pull", (STRPTR)"Pull", (STRPTR)"mmopPull", (STRPTR)"8EDA004B", TRUE },
	{ (STRPTR)"MMM_Seek", (STRPTR)"Seek", (STRPTR)"mmopSeek", (STRPTR)"8EDA0055", TRUE },
	{ (STRPTR)"MMM_GetPort", (STRPTR)"GetPort", (STRPTR)"mmopGetPort", (STRPTR)"8EDA0049", TRUE },
	{ (STRPTR)"MMM_SetPort", (STRPTR)"SetPort", (STRPTR)"mmopSetPort", (STRPTR)"8EDA004B", TRUE },
	{ NULL, NULL, NULL, NULL, FALSE }
};

///
/// strlow()

void strlow(STRPTR string)
{
	while (*string)
	{
		*string = ToLower(*string);
		string++;
	}
}

///
/// error()

void error(STRPTR msg, ...)
{
	STRPTR s;
	va_list args;

	va_start(args, msg);
	if ((s = VFmtNew(msg, args)) != NULL)
	{
		PutStr((STRPTR)s);
		StrFree(s);
	}
	va_end(args);
	return;
}

///

/// generate_dispatcher()

void generate_dispatcher(Object *obj, Object *method_list)
{
	ULONG method_number;

	TC("/// %sDispatcher()\n\n");
	TC("DISPATCHER(%sDispatcher)\n{\n");

	II; I; T("switch (msg->MethodID)\n");
	I; T("{\n"); II;

	for (method_number = 0;; method_number++)
	{
		struct MethodEntry *me;

		DoMethod(method_list, MUIM_List_GetEntry, method_number, (intptr_t)&me);
		if (!me) break;

		I; T("case "); T(me->Name); TC(":  return (%s"); T(me->Function); T("(cl, obj, ");

		if (*me->Structure)
		{
			T("(struct "); T(me->Structure); T("*)");
		}

		T("msg));\n");
	}

	I; T("default:  return (DoSuperMethodA(cl, obj, msg));\n");
	IO; I; T("}\n");
	IO; T("}\n\n///\n");
}

///
/// wait_loop()

void wait_loop(void)
{
  ULONG signals;

	while (DoMethod(App, MUIM_Application_NewInput, (intptr_t)&signals) != (ULONG)SUBWINDOW_CLOSE)
	{
		if (signals)
		{
			signals = Wait (signals | SIGBREAKF_CTRL_C);

			if (signals & SIGBREAKF_CTRL_C)
			{
				DoMethod(App, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
				break;
			}
		}
	}

	return;
}

///
/// labelled_checkmark

Object *labelled_checkmark(CONST_STRPTR label, IPTR id)
{
	Object *o;

	o = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Horiz, TRUE,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Image,
			ImageButtonFrame,
			MUIA_ShowSelState, FALSE,
			MUIA_Background, MUII_ButtonBack,
			MUIA_Image_Spec, "6:15",
			MUIA_InputMode, MUIV_InputMode_Toggle,
			MUIA_CycleChain, TRUE,
			MUIA_UserData, id,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Text_Contents, label,
			MUIA_Text_SetMax, TRUE,
		TAG_END),
	TAG_END);

	return o;
}

///
/// unexpected_eof()

void unexpected_eof(Object *obj, struct Parser *p)
{
	MUI_Request(App, obj, 0, "ChocolateCastle", (char*)LS(MSG_ERROR_REQUESTER_OK_BUTTON, "*_OK"),
	 (char*)LS(MSG_ERROR_REQUESTER_UNEXPECTED_EOF, "Unexpected end of file in line %lu."), p->line);
}

///
/// syntax_error()

void syntax_error(Object *obj, struct Parser *p)
{
	MUI_Request(App, obj, 0, "ChocolateCastle", (char*)LS(MSG_ERROR_REQUESTER_OK_BUTTON, "*_OK"),
	 (char*)LS(MSG_ERROR_REQUESTER_SYNTAX_ERROR, "Syntax error in line %lu."), p->line);
}

///

/* PROJECT FILE PARSING FUNCTIONS */

/// check_pattern()

BOOL check_pattern(STRPTR line, CONST_STRPTR pattern, struct Parser *parser)
{
	#if defined(__MORPHOS__)
	parser->args->RDA_Source.CS_Buffer = (UBYTE *)line;
	#else
	parser->args->RDA_Source.CS_Buffer = line;
	#endif
	parser->args->RDA_Source.CS_Length = strln(line);
	parser->args->RDA_Source.CS_CurChr = 0;
	parser->args->RDA_DAList = 0;
	parser->args->RDA_Buffer = NULL;
	parser->args->RDA_BufSiz = 0;
	parser->args->RDA_ExtHelp = NULL;
	parser->args->RDA_Flags = RDAF_NOPROMPT;
	if (ReadArgs(pattern, (LONG*)parser->params, parser->args)) return TRUE;
	return FALSE;
}

///



/// DoSuperNewM()

#if defined(__MORPHOS__)
Object* DoSuperNewM(Class *cl, Object *obj, ...)
{
	va_list args, args2;
	int argc = 0;
	uint32_t tag;
	intptr_t val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, obj);

	do
	{
		tag = va_arg(args, uint32_t);
		val = va_arg(args, intptr_t);
		argc++;
	}
	while (tag != TAG_MORE);

	va_end(args);

	{
		struct TagItem tags[argc];
		int i;

		va_start(args2, obj);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, uint32_t);
			tags[i].ti_Data = va_arg(args2, intptr_t);
		}

		va_end(args2);

		result = (Object*)DoSuperMethod(cl, obj, OM_NEW, (intptr_t)tags);
	}
	return result;
}
#else
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  Object *rc;
  VA_LIST args;

  VA_START(args, obj);
  #if defined(__AROS__)
  rc = (Object *)DoSuperNewTagList(cl, obj, NULL, (struct TagItem *)VA_ARG(args, IPTR));
  #else
  rc = (Object *)DoSuperMethod(cl, obj, OM_NEW, VA_ARG(args, ULONG), NULL);
  #endif
  VA_END(args);

  return rc;
}
#endif

///
/// MUI_NewObjectM()

#if defined(__MORPHOS__)
Object* MUI_NewObjectM(const char *classname, ...)
{
	va_list args, args2;
	int argc = 0;
	uint32_t tag;
	intptr_t val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, classname);

	while ((tag = va_arg(args, uint32_t)) != TAG_END)
	{
		val = va_arg(args, intptr_t);
		argc++;
	}

	va_end(args);

	{
		struct TagItem tags[argc + 1];  // one for {TAG_END, 0}
		int i;

		va_start(args2, classname);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, uint32_t);
			tags[i].ti_Data = va_arg(args2, intptr_t);
		}

		tags[argc].ti_Tag = TAG_END;
		tags[argc].ti_Data = 0;

		va_end(args2);

		result = (Object*)MUI_NewObjectA((STRPTR)classname, tags);
	}
	return result;
}
#endif

///
/// NewObjectM()

#if defined(__MORPHOS__)
Object* NewObjectM(Class *cl, const char *classname, ...)
{
	va_list args, args2;
	int argc = 0;
	uint32_t tag;
	intptr_t val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, classname);

	while ((tag = va_arg(args, uint32_t)) != TAG_END)
	{
		val = va_arg(args, intptr_t);
		argc++;
	}

	va_end(args);

	{
		struct TagItem tags[argc + 1];  // one for {TAG_END, 0}
		int i;

		va_start(args2, classname);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, uint32_t);
			tags[i].ti_Data = va_arg(args2, intptr_t);
		}

		tags[argc].ti_Tag = TAG_END;
		tags[argc].ti_Data = 0;

		va_end(args2);

		result = NewObjectA(cl, (STRPTR)classname, tags);
	}
	return result;
}
#endif

///
