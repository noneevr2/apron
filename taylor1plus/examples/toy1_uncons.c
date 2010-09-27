#include <time.h>
#include "ap_global1.h"
#include "t1p.h"
#include "oct.h"
#include "pk.h"
#include "box.h"
#include "ap_ppl.h"

/******************************
  void main() {
    float x,y;
      x = FBETWEEN(0,10);
        y = x*x-x;
	  if (y>=0)
	      y = x/10.0;
	        else
		    y = x*x + 2;
		    }
  *****************************/

int main(void) {
    clock_t start, end;
        double time;
	    double cpu_time_used;
	        start = clock();

    /* choisir le domaine pour les symboles de bruit.
       - box_manager_alloc() pour les intervalles
       - oct_manager_alloc() pour les octogones
       - pk_manager_alloc(true) pour les polyhedres version newpolka, (true means strict constraints are supported)
       - ap_ppl_poly_manager_alloc(true) pour les polyhedre version ppl
     */
    ap_manager_t* manNS = box_manager_alloc();
    /* Vous pouvez choisir l'un des domaine pr�c�dent pour avoir les r�sus dans les zones/poly ... */
    //ap_manager_t* man = pk_manager_alloc(true);
    ap_manager_t* man = t1p_manager_alloc(manNS);
    /* par exemple pour les octogones */

    /* Construite un environnement de 2 variables r��lles */
    ap_var_t name_of_dim[2] = {"x", "y"};
    ap_environment_t* env = ap_environment_alloc(NULL, 0, &name_of_dim[0], 2);

    /* Construite l'equivalent du x = DBETWEEN(0,180) et y = top */
    ap_interval_t** array = ap_interval_array_alloc(2);
    ap_interval_set_double(array[0], 0, 10);
    ap_interval_set_double(array[1], INT_MIN, INT_MAX);

    /* abstraire le tableau d'intervalles array en un objet T1+ */
    ap_abstract1_t abs = ap_abstract1_of_box(man, env, name_of_dim, array, 2);
    /* pour afficher l'objet abstrait */
    ap_abstract1_fprint(stdout, man, &abs);

    /* construire la contrainte (x \leq 1) and (x \geq -1) */
    ap_linexpr1_t linexp1 = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, (size_t)1);
    ap_linexpr1_set_cst_scalar_double(&linexp1, (double)0.0);
    ap_linexpr1_set_coeff_scalar_double(&linexp1, "y", (double)(1.0));
    ap_lincons1_t cons1 = ap_lincons1_make(AP_CONS_SUPEQ, &linexp1, NULL);
    ap_lincons1_array_t cons1_array = ap_lincons1_array_make(env, 1);
    ap_lincons1_array_set(&cons1_array, 0, &cons1);
    /* pour afficher la contrainte */
    //ap_lincons1_array_fprint(stdout, &cons1_array);
    ap_linexpr1_t linexp2 = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, (size_t)1);
    ap_linexpr1_set_cst_scalar_double(&linexp2, (double)0.0);
    ap_linexpr1_set_coeff_scalar_double(&linexp2, "y", (double)(-1.0));
    ap_lincons1_t cons2 = ap_lincons1_make(AP_CONS_SUPEQ, &linexp2, NULL);
    ap_lincons1_array_t cons2_array = ap_lincons1_array_make(env, 1);
    ap_lincons1_array_set(&cons2_array, 0, &cons2);

    ap_texpr1_t* y = ap_texpr1_binop(AP_TEXPR_SUB,
	    ap_texpr1_binop(AP_TEXPR_MUL,
		ap_texpr1_var(env, "x"),
		ap_texpr1_var(env, "x"),
		AP_RTYPE_DOUBLE,
		AP_RDIR_UP),
	    ap_texpr1_var(env, "x"),
	    AP_RTYPE_DOUBLE,
	    AP_RDIR_UP);

    ap_texpr1_t* y1 = ap_texpr1_binop(AP_TEXPR_DIV,
	    ap_texpr1_var(env, "x"),
	    ap_texpr1_cst_scalar_double(env, 10.0),
	    AP_RTYPE_DOUBLE,
	    AP_RDIR_UP);

    ap_texpr1_t* y2 = ap_texpr1_binop(AP_TEXPR_ADD,
	    ap_texpr1_cst_scalar_double(env, 2.0),
	    ap_texpr1_binop(AP_TEXPR_MUL,
		ap_texpr1_var(env, "x"),
		ap_texpr1_var(env, "x"),
		AP_RTYPE_DOUBLE,
		AP_RDIR_UP),
	    AP_RTYPE_DOUBLE,
	    AP_RDIR_UP);

    ap_box1_t gamma;

    fprintf(stdout,"/********* ASSIGN y = x*x-x; *********/\n");
    ap_abstract1_t abs1 = ap_abstract1_assign_texpr(man, false, &abs, "y", y, NULL);
    ap_abstract1_fprint(stdout, man, &abs1);

    fprintf(stdout,"/********** IF (y>=0) **************/\n");
    //ap_abstract1_t abs2 = ap_abstract1_meet_lincons_array(man, false, &abs1, &cons1_array);
    //ap_abstract1_fprint(stdout, man, &abs2);

    fprintf(stdout,"/******** ASSIGN du IF ***********/\n");
    ap_abstract1_t abs3 = ap_abstract1_assign_texpr(man, false, &abs, "y", y1, NULL);
    ap_abstract1_fprint(stdout, man, &abs3);

    fprintf(stdout,"/*************** ELSE *************/");
    //ap_abstract1_t abs4 = ap_abstract1_meet_lincons_array(man, false, &abs1, &cons2_array);
    //ap_abstract1_fprint(stdout, man, &abs4);

    fprintf(stdout,"/******** ASSIGN du ELSE ***********/\n");
    ap_abstract1_t abs5 = ap_abstract1_assign_texpr(man, false, &abs, "y", y2, NULL);
    //ap_abstract1_fprint(stdout, man, &abs5);

    fprintf(stdout,"/******** JOIN IF ELSE  ***********/\n");
    ap_abstract1_t abs6 = ap_abstract1_join(man, false, &abs3, &abs5);
    ap_abstract1_fprint(stdout, man, &abs6);

    /* pour afficher la concretisation intervalle de abs_res */
    gamma = ap_abstract1_to_box(man, &abs6);
    double inf = 0.0; double sup = 0.0;
    size_t i = 0;
    for (i=0; i<env->realdim; i++) {
    ap_double_set_scalar(&inf, gamma.p[i]->inf, GMP_RNDU);
    ap_double_set_scalar(&sup, gamma.p[i]->sup, GMP_RNDU);
    printf("%s : [%f,%f]\n", (char*)name_of_dim[i], inf, sup);
    }
    ap_box1_fprint(stdout, &gamma);
    ap_box1_clear(&gamma);

    /* lib�rer la m�moire utilis�e*/
    ap_abstract1_clear(man, &abs);
    ap_abstract1_clear(man, &abs1);
    //ap_abstract1_clear(man, &abs2);
    ap_abstract1_clear(man, &abs3);
    //ap_abstract1_clear(man, &abs4);
    ap_abstract1_clear(man, &abs5);
    ap_abstract1_clear(man, &abs6);

    ap_lincons1_array_clear(&cons1_array);
    ap_lincons1_array_clear(&cons2_array);

    ap_interval_array_free(array, 2);
    ap_manager_free(man);
    ap_manager_free(manNS);
    end = clock();
        time = ((double) (end - start)) / CLOCKS_PER_SEC;
	    printf("CpuTime: %.2f\n",time);
}
