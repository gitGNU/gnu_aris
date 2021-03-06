/* The GNU Aris program.

   Copyright (C) 2012 Ian Dunn.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "process.h"
#include "vec.h"
#include "list.h"
#include "var.h"
#include "sen-data.h"
#include "proof.h"
#include "aio.h"
#include "rules.h"
#include "config.h"
#include "interop-isar.h"

#ifdef ARIS_GUI
#include <gtk/gtk.h>
#include "app.h"
#include "aris-proof.h"
#include "rules-table.h"
#endif

// The options array for getopt_long.

static struct option const long_opts[] =
  {
    {"evaluate", no_argument, NULL, 'e'},
    {"premise", required_argument, NULL, 'p'},
    {"conclusion", required_argument, NULL, 'c'},
    {"rule", required_argument, NULL, 'r'},
    {"variable", required_argument, NULL, 'a'},
    {"text", no_argument, NULL, 't'},
    {"file", required_argument, NULL, 'f'},
    {"grade", required_argument, NULL, 'g'},
    {"isar", required_argument, NULL, 'i'},
    {"sexpr", required_argument, NULL, 's'},
    {"boolean", no_argument, NULL, 'b'},
    {"list", no_argument, NULL, 'l'},
    {"verbose", no_argument, NULL, 'v'},
    {"latex", required_argument, NULL, 'x'},
    {"version", no_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

// The structure for holding the argument flags.

struct arg_items {
  int verbose : 1;
  int evaluate : 1;
  int boolean : 1;
  char * file_name[256];
  char * grade_name[256];
  char * latex_name[256];
  char * conclusion;
  vec_t * prems;
  char rule[3];
  char * rule_file;
  vec_t * vars;
};

struct arg_flags {
  int verbose : 1;
  int evaluate : 1;
  int boolean : 1;
};

/* Lists the rules.
 *  input:
 *    none.
 *  output:
 *    none.
 */
void
list_rules ()
{
  printf ("Inference rules:\n");
  printf ("  mp - Modus Ponens\n");
  printf ("  ad - Addition\n");
  printf ("  sm - Simplification\n");
  printf ("  cn - Conjunction\n");
  printf ("  hs - Hypothetical Syllogism\n");
  printf ("  ds - Disjunctive Syllogism\n");
  printf ("  ex - Excluded Middle\n");
  printf ("  cd - Constructive Dilemma\n");
  printf ("\n");
  printf ("Equivalence Rules\n");
  printf ("  im - Implication\n");
  printf ("  dm - DeMorgan*\n");
  printf ("  as - Association*\n");
  printf ("  co - Commutativity*\n");
  printf ("  id - Idempotence*\n");
  printf ("  dt - Distribution*\n");
  printf ("  eq - Equivalence\n");
  printf ("  dn - Double Negation*\n");
  printf ("  ep - Exportation\n");
  printf ("  sb - Subsumption*\n");
  printf ("\n");
  printf ("Predicate Rules\n");
  printf ("  ug - Universal Generalization\n");
  printf ("  ui - Universal Instantiation\n");
  printf ("  eg - Existential Generalization\n");
  printf ("  ei - Existential Instantiation\n");
  printf ("  bv - Bound Variable\n");
  printf ("  nq - Null Quantifier\n");
  printf ("  pr - Prenex\n");
  printf ("  ii - Identity\n");
  printf ("  fv - Free Variable\n");
  printf ("\n");
  printf ("Miscellaneous Rules\n");
  printf ("  lm - Lemma\n");
  printf ("  sp - Subproof\n");
  printf ("  sq - Sequence Instantiation\n");
  printf ("  in - Induction\n");
  printf ("\n");
  printf ("Boolean Rules\n");
  printf ("  bi - Boolean Identity*\n");
  printf ("  bn - Boolean Negation*\n");
  printf ("  bd - Boolean Dominance*\n");
  printf ("  sn - Symbol Negation*\n");
  printf ("\n");
  printf ("* = This rule is available in boolean mode.\n");

  exit (EXIT_SUCCESS);
}

/* Prints the version information and exits.
 *  input:
 *    none.
 *  output:
 *    none.
 */
void
version ()
{
  printf ("%s - %s\n", PACKAGE_NAME, VERSION);
  printf ("Copyright (C) 2012 Ian Dunn.\n");
  printf ("License GPLv3: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n");
  printf ("This is free software: you are free to change and redistribute it.\n");
  printf ("There is NO WARRANTY, to the extent premitted by the law.\n");
  printf ("\n");
  printf ("Written by Ian Dunn.\n");
  exit (EXIT_SUCCESS);
}

/* Prints usage and exits.
 *  input:
 *    status - status to exit with.
 *  output:
 *    none.
 */
void
usage (int status)
{
  printf ("Usage: aris [OPTIONS]... [-f FILE] [-g FILE]\n");
  printf ("   or: aris [OPTIONS]... [-p PREMISE]... -r RULE -c CONCLUSION\n");
  printf ("\n");
  printf ("Options:\n");
  printf ("  -a, --variable=VARIABLE        Use VARIABLE as a variable.\n");
  printf ("                                  Place an '*' next to the variable to designate it as arbitrary.\n");
  printf ("  -b, --boolean                  Run Aris in boolean mode.\n");
  printf ("  -c, --conclusion=CONCLUSION    Set CONCLUSION as the conclusion.\n");
  printf ("  -e, --evaluate                 Run Aris in evaluation mode.\n");
  printf ("  -f, --file=FILE                Evaluate FILE.\n");
  //printf ("  -g, --grade=FILE               Grade file flag FILE against grade flag FILE.\n");
  printf ("  -l, --list                     List the available rules.\n");
  printf ("  -p, --premise PREMISE          Use PREMISE as a premise.\n");
  printf ("  -r, --rule RULE                Set RULE as the rule.\n");
  printf ("                                  Use 'lm:/path/to/file' to designate a file.\n");
  printf ("  -t, --text TEXT                Simply check the correctness of TEXT.\n");
  printf ("  -v, --verbose                  Print status and error messages.\n");
  printf ("  -x, --latex=FILE               Convert FILE to a LaTeX proof file.\n");
  printf ("  -h, --help                     Print this help and exit.\n");
  printf ("      --version                  Print the version and exit.\n");
  printf ("\n");
  printf ("Report %s bugs to %s\n", PACKAGE_NAME, PACKAGE_BUGREPORT);
  printf ("%s home page: <http://www.gnu.org/software/%s/>\n",
	  PACKAGE_NAME, PACKAGE);
  printf ("General help using GNU software: <http://www.gnu.org/gethelp/>\n");

  exit (status);
}

/* Checks argument text.
 *  input:
 *   arg_text - the text to check.
 *  output:
 *   0 if unsuccessful, 1 if successful, -1 on error.
 */
int
check_arg_text (unsigned char * arg_text)
{
  int ret_chk;

  ret_chk = check_text (arg_text);
  if (ret_chk == -1)
    return -1;

  switch (ret_chk)
    {
    case 0:
      break;
    case -2:
      fprintf (stderr, "Text Error - there are mismatched parentheses in the string - '%s'\n", arg_text);
      return 0;
    case -3:
      fprintf (stderr, "Text Error - there are invalid connectives in the string - '%s'\n", arg_text);
      return 0;
    case -4:
      fprintf (stderr, "Text Error - there are invalid quantifiers in the string - '%s'\n", arg_text);
      return 0;
    case -5:
      fprintf (stderr, "Text Error - there are syntactical errors in the string - '%s'\n", arg_text);
      return 0;
    }

  return 1;
}

/* Converts sentence data to a LaTeX line.
 *  input:
 *   sd - the sentence data to convert.
 *  output:
 *   The converted text, or NULL on error.
 */
char *
convert_sd_latex (sen_data * sd)
{
  char * out_str, * text;
  int out_pos, i;

  text = die_spaces_die (sd->text);
  if (!text)
    return NULL;

  main_conns = gui_conns;

  out_str = (char *) calloc (sd->depth * 6 + 1, sizeof (char));
  CHECK_ALLOC (out_str, NULL);

  out_pos = 0;

  for (i = 0; i < sd->depth; i++)
    out_pos += sprintf (out_str + out_pos, "\\quad ");

  for (i = 0; text[i]; i++)
    {
      if (i > 0 && i % 80 == 0)
	{
	  out_str = (char *) realloc (out_str, out_pos + 5);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\\\\n&");
	}

      if (!strncmp (text + i, AND, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\land ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, OR, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\lor ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, NOT, NL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\neg ");
	  i += NL - 1;
	  continue;
	}

      if (!strncmp (text + i, CON, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\rightarrow ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, BIC, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 18);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\leftrightarrow ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, UNV, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\forall ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, EXL, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 16);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\exists ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, TAU, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\top ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, CTR, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\bot ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, ELM, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, "\\in ");
	  i += CL - 1;
	  continue;
	}

      if (!strncmp (text + i, NIL, CL))
	{
	  out_str = (char *) realloc (out_str, out_pos + 8);
	  CHECK_ALLOC (out_str, NULL);
	  out_pos += sprintf (out_str + out_pos, " nil ");
	  i += CL - 1;
	  continue;
	}

      out_str = (char *) realloc (out_str, out_pos + 2);
      CHECK_ALLOC (out_str, NULL);
      out_str[out_pos++] = text[i];

    }

  out_str = (char *) realloc (out_str, out_pos + 1);
  CHECK_ALLOC (out_str, NULL);
  out_str[out_pos] = '\0';

  main_conns = cli_conns;
  
  return out_str;
}

/* Converts a proof into a LaTeX file.
 *  input:
 *   proof - the proof to convert.
 *   file - the file to write to.
 *  output:
 *   0 on success, -1 on error.
 */
int
convert_proof_latex (proof_t * proof, FILE * file)
{
  fprintf (file, "\\documentclass{article}\n");
  fprintf (file, "\\usepackage{amsmath}\n");
  fprintf (file, "\\usepackage{amsfonts}\n");
  fprintf (file, "\\begin{document}\n");
  fprintf (file, "\\newcommand{\\eline}{--------}\n");
  fprintf (file, "\\newcommand{\\prmline}[2]{\\text{#1.}&\\quad #2 &}\n");
  fprintf (file, "\\newcommand{\\stdline}[3]{\\text{#1.}&\\quad #2 & \\texttt{#3 }}\n");
  fprintf (file, "\n");

  fprintf (file, "{\\allowdisplaybreaks\n");
  fprintf (file, "  \\begin{align*}\n");

  item_t * ev_itr;
  char * text;
  int i;

  for (ev_itr = proof->everything->head; ev_itr; ev_itr = ev_itr->next)
    {
      sen_data * sd;
      sd = ev_itr->value;

      if (!sd->premise)
	break;

      text = convert_sd_latex (sd);
      if (!text)
	return -1;

      fprintf (file, "    \\prmline{%i}{%s}\\\\\n", sd->line_num, text);
      free (text);
    }

  fprintf (file, "    &\\eline\\\\\n");

  for (; ev_itr; ev_itr = ev_itr->next)
    {
      sen_data * sd;
      sd = ev_itr->value;

      text = convert_sd_latex (sd);
      if (!text)
	return -1;

      const char * rule = sd->subproof ? "assume" : rules_list[sd->rule];

      fprintf (file, "    \\stdline{%i}{%s}{%s} ", sd->line_num,
	       text, rule);

      if (!(sd->rule == RULE_EX || sd->rule == RULE_II
	    || sd->rule == RULE_SQ || sd->subproof))
	{
	  for (i = 0; sd->refs[i] != -1; i++)
	    {
	      fprintf (file, "(%i)", sd->refs[i]);
	      if (sd->refs[i + 1] != -1)
		fprintf (file, ",");
	    }
	}

      fprintf (file, "\\\\\n");
      free (text);
    }

  fprintf (file, "  \\end{align*}\n");
  fprintf (file, "}\n");

  fprintf (file, "\\end{document}\n");
  return 0;
}

/* Grades a single proof against a file.
 */
int
grade_file (FILE * g_file, proof_t * c_file)
{
  int ret_chk, grade;
  vec_t * rets;

  rets = init_vec (sizeof (int));
  if (!rets)
    return -1;

  ret_chk = proof_eval (c_file, rets, 0);
  if (ret_chk == -1)
    return -1;

  // Grades them out of five points:
  // 2 - Effort (evaluates properly)
  // 2 - Is similar to the correct file
  // 1 - Obtains the goal(s).

  if (rets->num_stuff >= (c_file->everything->num_stuff / 2))
    {
      grade = 1;
      if (rets->num_stuff == c_file->everything->num_stuff)
	grade = 2;
    }

  // Read through the grade file, and get the goal(s).
  unsigned char * goal;
  fscanf (g_file, "%s", &goal);

  if (c_file->goals)
    {
      item_t * g_itr, * ev_itr;
      for (g_itr = c_file->goals->head; g_itr; g_itr = g_itr->next)
	{
	  sen_data * g_sd;
	  g_sd = g_itr->value;
	  for (ev_itr = c_file->everything->head; ev_itr; ev_itr = ev_itr->next)
	    {
	      sen_data * e_sd;
	      e_sd = ev_itr->value;
	      if (!strcmp (e_sd->text, g_sd->text))
		break;
	    }
	  if (!ev_itr)
	    break;
	}
      if (!g_itr)
	grade++;
    }

  // Check the sequence of rules, making sure that they are met
  // in the file being graded.


  return grade;
}

/* Parses the supplied arguments.
 *  input:
 *   argc, argv - should be self-explanatory.
 *   ai - a pointer to an argument structure to get the arguments.
 *  output:
 *   0 on success.
 */
int
parse_args (int argc, char * argv[], struct arg_items * ai)
{
  int c;

  int cur_file, cur_grade, cur_latex;
  int opt_len;
  int c_ret;

  cur_file = cur_grade = cur_latex = 0;
  ai->verbose = ai->boolean = ai->evaluate = 0;
  ai->rule_file = NULL;
  for (c = 0; c < 256; c++)
    {
      ai->file_name[c] = NULL;
      ai->grade_name[c] = NULL;
      ai->latex_name[c] = NULL;
    }

  ai->prems = init_vec (sizeof (char*));
  ai->vars = init_vec (sizeof (variable));
  memset ((char *) ai->rule, 0, sizeof (char) * 3);

  //Only one conclusion and one rule can exist.
  //int got_conc = false, got_rule = false;
  int got_conc = 0, got_rule = 0;

  main_conns = cli_conns;

  while (1)
    {
      int opt_idx = 0;

      c = getopt_long (argc, argv, "ep:c:r:t:a:f:g:i:s:x:lbvh", long_opts, &opt_idx);

      if (c == -1)
	break;

      switch (c)
	{
	case 'e':
	  ai->evaluate = 1;
	  break;
	case 'p':
	  if (optarg)
	    {
	      unsigned char * sexpr_prem;
	      unsigned char * tmp_str;

	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1 || c_ret == 0)
		exit (EXIT_FAILURE);

	      tmp_str = die_spaces_die (optarg);
	      if (!tmp_str)
		exit (EXIT_FAILURE);

	      sexpr_prem = convert_sexpr (tmp_str);
	      if (!sexpr_prem)
		exit (EXIT_FAILURE);
	      free (tmp_str);

	      c_ret = vec_str_add_obj (ai->prems, sexpr_prem);
	      if (c_ret == -1)
		exit (EXIT_FAILURE);
	      free (sexpr_prem);
	      break;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - premise flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'c':

	  if (optarg)
	    {
	      if (got_conc)
		{
		  fprintf (stderr, "Argument Error - only one (1) conclusion must be specified, ignoring conclusion \"%s\".\n", optarg);
		  break;
		}

	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1 || c_ret == 0)
		exit (EXIT_FAILURE);

	      unsigned char * tmp_str;
	      tmp_str = die_spaces_die (optarg);
	      if (!tmp_str)
		exit (EXIT_FAILURE);

	      ai->conclusion = convert_sexpr (tmp_str);
	      if (!ai->conclusion)
		exit (EXIT_FAILURE);
	      free (tmp_str);

	      got_conc = 1;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - conclusion flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 't':
	  if (optarg)
	    {
	      c_ret = check_arg_text (optarg);
	      if (c_ret == -1)
		exit (EXIT_FAILURE);
	      printf ("Correct!\n");
	      exit (EXIT_SUCCESS);
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - text flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'r':

	  if (optarg)
	    {
	      if (got_rule)
		{
		  fprintf (stderr, "Argument Warning - only one (1) rule must be specified, ignoring rule \"%s\".\n", optarg);
		  break;
		}

	      opt_len = strlen (optarg);

	      if (opt_len > 2 && strncmp (optarg, "lm:", 3))
		{
		  fprintf (stderr, "Argument Warning - a rule must be two (2) characters long, ignoring rule \"%s\".\n", optarg);
		  break;
		}

	      strncpy (ai->rule, optarg, 2);
	      ai->rule[2] = '\0';

	      if (strlen (optarg) > 3)
		{
		  ai->rule_file = (char *) calloc (opt_len - 2, sizeof (char));

		  strncpy (ai->rule_file, optarg + 3, opt_len - 3);
		  ai->rule_file[opt_len - 3] = '\0';
		}
	      got_rule = 1;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - rule flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'a':
	  if (optarg)
	    {
	      if (!islower (optarg[0]))
		{
		  fprintf (stderr, "Argument Warning - the first character of a variable must be lowercase, ignoring variable \"%s\".\n", optarg);
		  break;
		}

	      variable v;
	      opt_len = strlen (optarg);

	      v.text = (unsigned char *) calloc (opt_len, sizeof (char));
	      strcpy (v.text, optarg);

	      if (v.text[opt_len - 1] == '*')
		{
		  v.arbitrary = 1;
		  v.text[opt_len - 1] = '\0';
		}
	      else
		{
		  v.arbitrary = 0;
		}

	      vec_add_obj (ai->vars, &v);
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - variable flag requires an argument, ignoring flag.\n");
	      break;
	    }
	  break;

	case 'f':

	  if (optarg)
	    {
	      if (ai->file_name[255])
		{
		  fprintf (stderr, "Argument Warning - a maximum of 256 filenames can be specified, ignoring file \"%s\".\n", optarg);
		  break;
		}

	      int arg_len = strlen (optarg);

	      ai->file_name[cur_file] = (char *) calloc (arg_len + 1,
							 sizeof (char));
	      if (!ai->file_name[cur_file])
		{
		  perror (NULL);
		  exit (EXIT_FAILURE);
		}

	      strcpy (ai->file_name[cur_file], optarg);
	      cur_file++;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - file flag requires a filename, ignoring flag.\n");
	      break;
	    }
	  break;

	  /*
	case 'g':
	  if (optarg)
	    {
	      if (ai->grade_name[255])
		{
		  fprintf (stderr, "Argument Warning - a maximum of 256 filenames can be specified, ignoring file \"%s\".\n", optarg);
		  break;
		}

	      int arg_len = strlen (optarg);

	      ai->grade_name[cur_grade] = (char *) calloc (arg_len + 1,
							   sizeof (char));
	      if (!ai->grade_name[cur_grade])
		{
		  perror (NULL);
		  exit (EXIT_FAILURE);
		}

	      strcpy (ai->grade_name[cur_grade], optarg);
	      cur_grade++;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - grade flag requires a filename, ignoring flag.\n");
	    }
	  break;
	  */

	case 'i':
	  if (optarg)
	    {
	      proof_t * proof;

	      proof = proof_init ();
	      if (!proof)
		exit (EXIT_FAILURE);

	      main_conns = cli_conns;
	      parse_thy (optarg, proof);
	      exit (EXIT_SUCCESS);
	    }

	case 's':
	  if (optarg)
	    {
	      main_conns = cli_conns;

	      unsigned char * no_spaces, * sexpr_str;
	      no_spaces = die_spaces_die (optarg);
	      sexpr_str = convert_sexpr (no_spaces);
	      printf ("%s\n", sexpr_str);
	      exit (EXIT_SUCCESS);
	    }

	case 'b':
	  ai->boolean = 1;
	  break;

	case 'l':
	  list_rules ();
	  break;

	case 'v':
	  ai->verbose = 1;
	  break;

	case 'x':
	  if (optarg)
	    {
	      if (ai->latex_name[255])
		{
		  fprintf (stderr, "Argument Warning - a maximum of 256 filenames can be specified, ignoring file \"%s\".\n", optarg);
		  break;
		}

	      int arg_len = strlen (optarg);

	      ai->latex_name[cur_latex] = (char *) calloc (arg_len + 1,
							   sizeof (char));
	      if (!ai->latex_name[cur_latex])
		{
		  perror (NULL);
		  exit (EXIT_FAILURE);
		}

	      strcpy (ai->latex_name[cur_latex], optarg);
	      cur_latex++;
	    }
	  else
	    {
	      fprintf (stderr, "Argument Warning - grade flag requires a filename, ignoring flag.\n");
	    }
	  break;

	case 0:
	  if (opt_idx == 14)
	    version ();
	  break;

	case 'h':
	  usage (EXIT_SUCCESS);
	  break;

	default:
	  fprintf (stderr, "Argument Error - ignoring unrecognized option: \"%c\" .\n",c);
	  break;
	}
    }

  return 0;
}

/* Main function.  There REALLY shouldn't be any explaination needed. */
int
main (int argc, char *argv[])
{
  int c;

  vec_t * prems;
  unsigned char * conc = NULL;
  char * rule;
  vec_t * vars;
  char ** file_name, ** grade_name, ** latex_name;
  proof_t ** proof, ** grade;
  int cur_file, cur_grade, cur_latex;
  char * rule_file = NULL;
  int opt_len, verbose, boolean, evaluate_mode;
  int c_ret;
  struct arg_items args;

  c_ret = parse_args (argc, argv, &args);

  prems = args.prems;
  conc = args.conclusion;
  vars = args.vars;
  verbose = args.verbose;
  evaluate_mode = args.evaluate;
  boolean = args.boolean;
  file_name = args.file_name;
  grade_name = args.grade_name;
  latex_name = args.latex_name;
  rule = args.rule;
  rule_file = args.rule_file;

  cur_file = cur_grade = cur_latex = -1;

  cur_grade = 0;
  for (c = 0; c < 256; c++)
    {
      if (cur_file != -1 && cur_grade != -1 && cur_latex != -1)
	break;

      if (cur_file == -1 && !file_name[c])
	cur_file = c;

      if (cur_grade == -1 && !grade_name[c])
	cur_grade = c;

      if (cur_latex == -1 && !latex_name[c])
	cur_latex = c;
    }

  if (conc == NULL && evaluate_mode && !file_name[0] && !latex_name[0])
    {
      fprintf (stderr, "Argument Error - a conclusion must be specified in evaluation mode.\n");
      exit (EXIT_FAILURE);
    }

  if (cur_latex > 0)
    {
      proof = (proof_t **) calloc (cur_latex, sizeof (proof_t *));
      CHECK_ALLOC (proof, EXIT_FAILURE);

      for (c = 0; c < cur_latex; c++)
	{
	  proof[c] = aio_open (latex_name[c]);
	  if (!proof[c])
	    exit (EXIT_FAILURE);

	  FILE * file;
	  char * fname;
	  int n_len;

	  n_len = strlen (latex_name[c]);
	  fname = (char *) calloc (n_len + 1, sizeof (char));
	  CHECK_ALLOC (fname, EXIT_FAILURE);

	  strncpy (fname, latex_name[c], n_len - 3);
	  sprintf (fname + n_len - 3, "tex");

	  file = fopen (fname, "w+");
	  if (!file)
	    {
	      perror (NULL);
	      exit (EXIT_FAILURE);
	    }

	  c_ret = convert_proof_latex (proof[c], file);
	  if (c_ret == -1)
	    exit (EXIT_FAILURE);

	  proof_destroy (proof[c]);
	  free (proof[c]);
	}

      exit (EXIT_SUCCESS);
    }

  if (cur_file > 0)
    {
      proof = (proof_t **) calloc (cur_file, sizeof (proof_t *));
      if (!proof)
	{
	  perror (NULL);
	  exit (EXIT_FAILURE);
	}

      for (c = 0; c < cur_file; c++)
	{
	  proof[c] = aio_open (file_name[c]);
	  if (!proof[c])
	    exit (EXIT_FAILURE);
	}
    }

  if (/*cur_grade > 0*/0)
    {
      grade = (proof_t **) calloc (cur_grade, sizeof (proof_t *));
      if (!grade)
	{
	  perror (NULL);
	  exit (EXIT_FAILURE);
	}

      /*
      for (c = 0; c < cur_grade; c++)
	{
	  grade[c] = aio_open (grade_name[c]);
	  if (!grade[c])
	    exit (EXIT_FAILURE);
	}
      */
    }

  if (evaluate_mode)
    {
      main_conns = cli_conns;

      if (cur_file > 0)
	{
	  main_conns = gui_conns;

	  /*TODO: Figure out how we are going to handle the grade flag. */
	  if (/*cur_grade > 0*/0)
	    {
	      // Maybe only allow one file per set of grade flags.
	      if (cur_file < 1)
		{
		  fprintf (stderr, "Argument Error - a file flag must be used with the grade flag.\n");
		  exit (EXIT_FAILURE);
		}

	      int g;
	      FILE * g_file;
	      g_file = fopen (grade_name[0], "r+");
	      if (!g_file)
		{
		  exit (EXIT_FAILURE);
		}

	      for (c = 0; c < cur_file; c++)
		{
		  g = grade_file (g_file, proof[c]);
		  if (g == -1)
		    exit (EXIT_FAILURE);
		  printf ("Graded File %i. %s => Grade = %i\n", c, grade_name[c], g);
		}
	    }

	  for (c = 0; c < cur_file; c++)
	    {
	      int ret_chk;
	      ret_chk = proof_eval (proof[c], NULL, verbose);
	      if (ret_chk == -1)
		exit (EXIT_FAILURE);
	    }
	}
      else
	{
	  char * p_ret;

	  proof_t * proof = NULL;

	  if (rule_file)
	    {
	      int f_len;
	      f_len = strlen (rule_file);
	      if (!strcmp (rule_file + f_len - 4, ".thy"))
		{
		  int ret_chk;
		  proof = proof_init ();
		  if (!proof)
		    exit (EXIT_FAILURE);

		  ret_chk = parse_thy (rule_file, proof);
		  if (ret_chk == -1)
		    exit (EXIT_FAILURE);
		}
	      else
		{
		  main_conns = gui_conns;
		  proof = aio_open (rule_file);
		  if (!proof)
		    exit (EXIT_FAILURE);
		  main_conns = cli_conns;
		}
	    }

	  p_ret = process (conc, prems, rule, vars, proof);
	  if (!p_ret)
	    exit (EXIT_FAILURE);

	  printf ("%s\n", p_ret);
	}

      return 0;
    }
  else
    {
#ifndef ARIS_GUI
      fprintf (stderr, "Fatal Error - evaluate flag not specified in non-gui mode.\n");
      exit (EXIT_FAILURE);
#else

      main_conns = gui_conns;

      gtk_init (&argc, &argv);

      the_app = init_app (boolean, verbose);

      // Get the current working directory from arg0, then determine the help file.
      GFile * arg0, * parent;

      arg0 = g_file_new_for_commandline_arg (argv[0]);
      parent = g_file_get_parent (arg0);
      the_app->working_dir = g_file_get_path (parent);

      if (the_app->working_dir)
	{
	  parent = g_file_get_parent (parent);
	  sprintf (the_app->help_file, "file://%s/doc/aris/index.html", g_file_get_path (parent));
	}

      int ret;
      if (cur_file > 0)
	{

	  for (c = 0; c < cur_file; c++)
	    {
	      aris_proof * new_gui = aris_proof_init_from_proof (proof[c]);
	      if (!new_gui)
		return -1;

	      aris_proof_set_filename (new_gui, file_name[c]);
	      new_gui->edited = 0;

	      ret = the_app_add_gui (new_gui);
	      if (ret < 0)
		return -1;
	    }

	  gtk_widget_show_all (the_app->rt->window);
	  rules_table_align (the_app->rt, the_app->focused);
	  rules_table_set_font (the_app->rt, FONT_TYPE_SMALL);
	}
      else
	{
	  aris_proof * main_gui;
	  main_gui = aris_proof_init ();
	  if (!main_gui)
	    return -1;

	  ret = the_app_add_gui (main_gui);
	  if (ret < 0)
	    return -1;

	  gtk_widget_show_all (the_app->rt->window);
	  rules_table_align (the_app->rt, main_gui);
	  rules_table_set_font (the_app->rt, FONT_TYPE_SMALL);
	}
      gtk_main ();
#endif
    }

  return 0;
}
