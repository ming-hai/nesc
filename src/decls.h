/* This file is part of the nesC compiler.

This file is derived from the RC Compiler. It is thus
   Copyright (C) 2000-2001 The Regents of the University of California.
Changes for nesC are
   Copyright (C) 2002 Intel Corporation

The attached "nesC" software is provided to you under the terms and
conditions of the GNU General Public License Version 2 as published by the
Free Software Foundation.

nesC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with nesC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA. */

#ifndef DECLS_H
#define DECLS_H

#include "AST.h"
#include "env.h"

typedef struct environment *environment;

#include "nesc-decls.h"

/* Types representing declarations */

typedef struct field_declaration {
  struct field_declaration *next; /* Next field in struct/union */
  const char *name;		/* May be NULL for bitfields (if NULL, bitwidth == 0) */
  type type;
  field_decl ast;
  int bitwidth;			/* for bitfields, -1 otherwise */
  size_t offset;		/* in bits, not bytes */
  bool offset_cc;		/* TRUE if offset is a compile-time constant
				   (can be false because of variable-size arrays) */
  bool packed;			/* if packed attribute specified */
} *field_declaration;

/* A struct, union or enum */
typedef struct tag_declaration {
  int kind; /* One of kind_{struct/union/enum}_ref */
  const char *name; /* NULL for anonynous struct/union/enum */
  type reptype; /* The type used to represent an enum, NULL for struct
		   and unions */
  /* fields and fieldlist are only defined for structs/unions */
  env fields;
  field_declaration fieldlist;
  tag_ref ast;
  struct tag_declaration *shadowed; /* Any struct with the same tag defined in enclosing scope */
  bool defined, being_defined;
  bool fields_const, fields_volatile;
  bool transparent_union;	/* transparent_union attribute is present */

  largest_uint size;
  size_t alignment;
  bool size_cc;			/* TRUE if size is a compile-time constant
				   (can be false because of variable-size arrays) */
  bool packed;			/* if packed attribute specified */

  nesc_declaration container;	/* as in data_declarations */
} *tag_declaration;

typedef enum { decl_variable, decl_constant, decl_function,
	       decl_typedef, decl_error, decl_magic_string,	
	       decl_magic_function,
	       decl_interface_ref, decl_component_ref } data_kind;

typedef struct data_declaration {
  data_kind kind;
  const char *name;
  type type;
  /* For declaration numbering purposes. At this point, it has:
     a per-function numbering for local variables
     a per-module numbering for commands/events */
  long id;
 
  /* Regular C: For extern's shadowing globals in inner scopes */
  /* nesC commands/events: point to original interface declaration */
  struct data_declaration *shadowed;

  /* interface/module/configuration this declaration belongs to.
     NULL for declarations from C files */
  nesc_declaration container;

  declaration definition; /* Pointer to actual definition, if any */
  declaration ast; /* Last declaration */
  expression initialiser; /* NULL if none */

  bool islimbo; /* TRUE if comes from an extern declaration in an inner scope
		   (also true for implicit function declarations) */
  bool isexternalscope; /* == TREE_PUBLIC   */
  bool isfilescoperef; /* == DECL_EXTERNAL */
  bool needsmemory;   /* == TREE_STATIC   */

  /* isused is TRUE if declaration used. For parameters, there is a special
     use during parameter list declaration to support forward parameters:
       - a duplicate parameter declaration is allowed if isused is FALSE
         once a duplicate is seen, isused is set to TRUE
       - parameters are created with isused == TRUE
       - after the forward parameters are seen, they have their isused field
         set to FALSE */
  bool isused;
  bool in_system_header;
  bool Cname;			/* name is in C name space (don't rename!)
				   Set by the `C' attribute. */
  bool spontaneous;		/* TRUE if called by environment (main,
				   interrupt handlers, e.g.). Set by the 
				   `spontaneous' attribute */

  /* For functions */
  enum { function_implicit, function_normal, function_static, function_nested,
         function_event, function_command }
    ftype;
  bool isinline;
  bool isexterninline;
  bool defined;			/* nesC: true if defined, false if used */
  bool suppress_definition;	/* Prevent code generation */
  bool uncallable;		/* Error if called */
  struct data_declaration *interface;	/* nesC: interface this cmd/event belongs to */
  typelist oldstyle_args; /* Type of arguments from old-style declaration */
  dd_list uses;			/* list of declarations of identifiers used */
  struct connections *connections; /* See nesc-generate.c: what this command
				      or event is connected to. */
  /* reduction function for magic functions */
  expression (*magic_reduce)(function_call fcall);

  /* For variables */
  enum { variable_register, variable_static, variable_normal } vtype;
  bool islocal;
  bool isparameter; /* implies islocal */
  bool isgeneric; /* nesc: implies isparameter, for the generic parameters of
		     commands and events */

  /* For constants */
  known_cst value;

  /* For magic_strings */
  const wchar_t *chars;
  size_t chars_length;

  /* For interface_ref */
  interface_declaration itype;
  environment functions;
  bool required;
  typelist gparms;

  /* For component_ref */
  component_declaration ctype;

} *data_declaration;

typedef struct label_declaration {
  const char *name;
  bool explicitly_declared;
  bool used;
  id_label firstuse; /* Never NULL */
  id_label definition; /* NULL until actually defined */
  function_decl containing_function;
} *label_declaration;

struct environment
{
  struct environment *sameregion parent;
  function_decl fdecl;
  bool parm_level : 1;
  bool global_level : 1;	/* Both system and component */
  env sameregion id_env;
  env sameregion tag_env;
};

extern data_declaration bad_decl;

#endif
