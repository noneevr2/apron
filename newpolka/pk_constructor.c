/* ********************************************************************** */
/* pk_constructor.c: constructors and accessors */
/* ********************************************************************** */

/* This file is part of the APRON Library, released under LGPL license.  Please
   read the COPYING file packaged in the distribution */

#include "pk_internal.h"
#include "eitvMPQ.h"
#include "ap_generic.h"

/* ********************************************************************** */
/* I. Constructors */
/* ********************************************************************** */

/* ====================================================================== */
/* Empty polyhedron */
/* ====================================================================== */

void poly_set_bottom(pk_internal_t* pk, pk_t* po)
{
  if (po->C) matrix_free(po->C);
  if (po->F) matrix_free(po->F);
  if (po->satC) satmat_free(po->satC);
  if (po->satF) satmat_free(po->satF);
  po->C = po->F = NULL;
  po->satC = po->satF = NULL;
  po->status = pk_status_conseps | pk_status_minimaleps;
  po->nbeq = po->nbline = 0;
}

/*
The empty polyhedron is just defined by the absence of both
constraints matrix and frames matrix.
*/

pk_t* pk_bottom(ap_manager_t* man, ap_dimension_t dim)
{
  pk_t* po = poly_alloc(dim);
  pk_internal_t* pk = pk_init_from_manager(man,AP_FUNID_BOTTOM);
  pk_internal_realloc_lazy(pk,dim.intd+dim.reald);
  po->status = pk_status_conseps | pk_status_minimaleps;
  man->result.flag_exact = man->result.flag_best = true;
  return po;
}

/* ====================================================================== */
/* Universe polyhedron */
/* ====================================================================== */

void matrix_fill_constraint_top(pk_internal_t* pk, matrix_t* C, size_t start)
{
  if (pk->strict){
    /* constraints epsilon and xi-epsilon*/
    vector_clear(C->p[start+0],C->nbcolumns);
    vector_clear(C->p[start+1],C->nbcolumns);
    numintMPQ_set_int(C->p[start+0][0],1);
    numintMPQ_set_int(C->p[start+0][polka_eps],1);
    numintMPQ_set_int(C->p[start+1][0],1);
    numintMPQ_set_int(C->p[start+1][polka_cst],1);
    numintMPQ_set_int(C->p[start+1][polka_eps],(-1));
  }
  else {
    /* constraint \xi \geq 0 */
    vector_clear(C->p[start+0],C->nbcolumns);
    numintMPQ_set_int(C->p[start+0][0],1);
    numintMPQ_set_int(C->p[start+0][polka_cst],1);
  }
}

void poly_set_top(pk_internal_t* pk, pk_t* po)
{
  size_t i;
  size_t dim;

  if (po->C) matrix_free(po->C);
  if (po->F) matrix_free(po->F);
  if (po->satC) satmat_free(po->satC);
  if (po->satF) satmat_free(po->satF);

  po->status =
    pk_status_conseps |
    pk_status_consgauss |
    pk_status_gengauss |
    pk_status_minimaleps
    ;

  dim = po->dim.intd + po->dim.reald;

  po->C = matrix_alloc(pk->dec-1, pk->dec+dim,true);
  po->F = matrix_alloc(pk->dec+dim-1,pk->dec+dim,true);
  /* We have to ensure that the matrices are really sorted */
  po->satC = satmat_alloc(pk->dec+dim-1,bitindex_size(pk->dec-1));
  po->satF = 0;
  po->nbeq = 0;
  po->nbline = dim;

  /* constraints */
  matrix_fill_constraint_top(pk,po->C,0);

  /* generators */
  /* lines $x_i$ */
  for(i=0; i<dim; i++){
    numintMPQ_set_int(po->F->p[i][pk->dec+dim-1-i],1);
  }
  if (pk->strict){
    /* rays xi and xi+epsilon */
    numintMPQ_set_int(po->F->p[dim][0],1);
    numintMPQ_set_int(po->F->p[dim][polka_cst],1);
    numintMPQ_set_int(po->F->p[dim+1][0],1);
    numintMPQ_set_int(po->F->p[dim+1][polka_cst],1);
    numintMPQ_set_int(po->F->p[dim+1][polka_eps],1);
    /* saturation matrix */
    po->satC->p[dim][0] = bitstring_msb >> 1;
    po->satC->p[dim+1][0] = bitstring_msb;
  }
  else {
    /* ray xi */
    numintMPQ_set_int(po->F->p[dim][0],1);
    numintMPQ_set_int(po->F->p[dim][polka_cst],1);
    /* saturation matrix */
    po->satC->p[dim][0] = bitstring_msb;
  }
  assert(poly_check(pk,po));
}

pk_t* pk_top(ap_manager_t* man, ap_dimension_t dim)
{
  pk_t* po;
  pk_internal_t* pk = pk_init_from_manager(man,AP_FUNID_TOP);
  pk_internal_realloc_lazy(pk,dim.intd+dim.reald);

  po = poly_alloc(dim);
  poly_set_top(pk,po);
  man->result.flag_exact = man->result.flag_best = true;
  return po;
}

/* ====================================================================== */
/* Hypercube polyhedron */
/* ====================================================================== */


/* The matrix is supposed to be big enough */
static 
int matrix_fill_constraint_box(
    pk_internal_t* pk,
    matrix_t* C, size_t start, ap_box0_t box, ap_dimension_t dim, bool integer)
{
  size_t k;
  ap_dim_t i;
  bool ok;
  eitvMPQ_t eitv;
  ap_coeff_t coeff;

  k = start;
  eitvMPQ_init(eitv);
  for (i=0; i<dim.intd+dim.reald; i++){
    ap_box0_ref_index(coeff,box,i);
    eitvMPQ_set_ap_coeff(eitv,coeff,pk->num);
    if (eitvMPQ_is_point(eitv)){
      ok = vector_set_dim_bound(pk,C->p[k],
				 (ap_dim_t)i, boundMPQ_numref(eitv->itv->sup), 0,
				 dim,
				 integer);
      if (!ok){
	eitvMPQ_clear(eitv);
	return -1;
      }
      k++;
    }
    else {
      /* inferior bound */
      if (!boundMPQ_infty(eitv->itv->neginf)){
	vector_set_dim_bound(pk,C->p[k],
			     (ap_dim_t)i, boundMPQ_numref(eitv->itv->neginf), -1,
			     dim,
			     integer);
	k++;
      }
      /* superior bound */
      if (!boundMPQ_infty(eitv->itv->sup)){
	vector_set_dim_bound(pk,C->p[k],
			     (ap_dim_t)i, boundMPQ_numref(eitv->itv->sup), 1,
			     dim,
			     integer);
	k++;
      }
    }
  }
  eitvMPQ_clear(eitv);
  return (int)k;
}

/* Abstract an hypercube defined by the array of intervals of size
   dim.intd+dim.reald.  */

pk_t* pk_of_box(ap_manager_t* man,
		ap_dimension_t dim,
		ap_box0_t box)
{
  int k;
  size_t nbdims;
  pk_t* po;

  pk_internal_t* pk = pk_init_from_manager(man,AP_FUNID_OF_BOX);
  pk_internal_realloc_lazy(pk,dim.intd+dim.reald);

  nbdims = dim.intd + dim.reald;
  po = poly_alloc(dim);
  po->status = pk_status_conseps;

  po->C = matrix_alloc(pk->dec-1 + 2*nbdims, pk->dec + nbdims, false);

  /* constraints */
  matrix_fill_constraint_top(pk,po->C,0);
  k = matrix_fill_constraint_box(pk,po->C,pk->dec-1,box,dim,true);
  if (k==-1){
    matrix_free(po->C);
    po->C = NULL;
    return po;
  }
  po->C->nbrows = (size_t)k;
  
  assert(poly_check(pk,po));
  man->result.flag_exact = man->result.flag_best = true;
  return po;
}

/* ********************************************************************** */
/* II. Accessors */
/* ********************************************************************** */

/* Return the dimensions of the polyhedra */
ap_dimension_t pk_dimension(ap_manager_t* man, pk_t* po){
  ap_dimension_t res;
  res.intd = po->dim.intd;
  res.reald = po->dim.reald;
  return res;
}

