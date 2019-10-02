/* file zdc.c; copyright Dimitre Trendafilov (2003)
 * command line frontend for computing zdelta difference
 * takes from 1 to 3 command line parameters
 * argv[1] - reference file 
 * argv[2] - target file; optional - if not specified read from stdin
 * argv[3] - delta file; optional - if not specified write to stdout
 * if K>3 arguments, the first K-2 are considered reference files
 */

#include <stdio.h>
#include <string.h>
#include "zd_mem.h"     /* dynamic memory allocation */
#include "zdlib.h"      /* zdelta interface          */

static const char usage_single[] = "usage: zdelta [diff|patch] Reference [Target [Delta]]";
static const char usage_multi[]  = "       zdelta [diff|patch] Reference1 Reference2... Target Delta";

#if REFNUM > 1
#define ZDDiffMulti(argc,argv) zd_diff_multi(argc,argv)
#define ZDPatchMulti(argc,argv) zd_patch_multi(argc,argv)
#else
#define ZDDiffMulti(argc, argv) 0
#define ZDPatchMulti(argc, argv) 0
#endif

/*===========================================================================
 *  multiple refernce files
 */

#if REFNUM > 1
int zd_diff_multi(int argc, char *argv[]){
  zd_mem_buffer tar_buf;                 /* target memory buffer     */
  FILE         *fp_ref[REFNUM];          /* reference file handles   */
  zd_mem_buffer ref_buf[REFNUM];         /* reference target buffers */
  Bytef        *ref[REFNUM];
  uLong         ref_s[REFNUM];
  int           ref_n, i;
  int           input=2, output=3;

  Bytef         *delta = NULL;
  uLong         d_size = 0;

  ref_n  = argc - 4; /* get the number of reference files */
  input  = argc - 2; /* get the input argument index      */
  output = argc - 1; /* get the output argument index     */

  /* open and check the input files */
  for(i=0;i<ref_n;++i){
    if( !(fp_ref[i] = fopen(argv[i+2],"rb")) ){ perror(argv[i+1]); exit(0); }
  }
  if(!freopen(argv[input],"rb",stdin)){ perror(argv[input]); exit(0); }

  /* copy input data to memory */
  for(i=0;i<ref_n;++i){
    if(dread_file(fp_ref[i],&ref_buf[i]) < 0){ perror(argv[i]); exit(0); }
    ref[i] = ref_buf[i].buffer;  ref_s[i] = ref_buf[i].pos-ref_buf[i].buffer; 
  }
  if(dread_file(stdin,&tar_buf) < 0) { perror(argv[input]); exit(0); }

  /* compress the data */
  if(zd_compressN1((const Bytef**) ref, ref_s, ref_n, 
		   (const Bytef*) tar_buf.buffer, tar_buf.pos - tar_buf.buffer,
		   &delta, &d_size) == ZD_OK){
    
    /* successfull compression write the delta to a file */
    if(!freopen(argv[output],"wb",stdout)){
      perror(argv[output]);
      exit(0);
    }

    if(d_size !=  fwrite(delta,sizeof(char),d_size ,stdout)) perror("ouput");
  }

  /* release memory */
  free(delta);
  zd_free(&tar_buf);
  for(i=ref_n-1;i>=0;--i){
    zd_free(&ref_buf[i]); /* free  ref. buffers */
    fclose(fp_ref[i]);    /* close ref files    */ 
  }

  return 0;
}
#endif /* REFNUM */

/*===========================================================================
 *  single reference file
 */

int zd_diff_single(int argc, char *argv[]){
  FILE *fp_ref;
  zd_mem_buffer tar_buf;
  zd_mem_buffer ref_buf;
  Bytef *delta = NULL;
  uLong d_size = 0;

  /* open and check the input files */
  if( !(fp_ref = fopen(argv[2],"rb")) )
  {
    perror(argv[2]);
    exit(0);
  }

  if(argc>3 && (!freopen(argv[3],"rb",stdin)))
  {
    perror(argv[3]); 
    exit(0);
  }

  /* copy input data to memory */
  if(dread_file(fp_ref,&ref_buf) < 0) perror(argv[1]);
  if(dread_file(stdin,&tar_buf) < 0) perror(argv[2]);

  /* compress the data */
  if(zd_compress1((const Bytef*) ref_buf.buffer, ref_buf.pos - ref_buf.buffer,
		  (const Bytef*) tar_buf.buffer, tar_buf.pos - tar_buf.buffer,
		  &delta, &d_size) == ZD_OK){

    /* successfull compression write the delta to a file */
    if(argc>4 && (!freopen(argv[4],"wb",stdout))){
      perror(argv[4]);
      exit(0);
    }
    if(d_size != fwrite(delta, sizeof(char), d_size, stdout)) perror("ouput");
  }
  
  /* release memory */
  free(delta);
  zd_free(&tar_buf);
  zd_free(&ref_buf);

  /* close files */
  fclose(fp_ref);

  return 0;
}

/*===========================================================================
 *  multiple reference files
 */
#if REFNUM>1
int zd_patch_multi(int argc, char *argv[]){

  FILE *fp_ref[REFNUM];
  zd_mem_buffer ref_buf[REFNUM];
  Bytef *ref[4];
  uLong ref_s[4];
  int   ref_n,i;

  zd_mem_buffer delta;
  Bytef *tar_buf = NULL;
  uLong t_size = 0;
  int   input, output;

  ref_n  = argc - 4; /* get the number of reference files */
  input  = argc - 2; /* get the input argument index */
  output = argc - 1; /* get the output argument index */

  /* open and check the input files */
  for(i=0;i<ref_n;++i){
    if( !(fp_ref[i] = fopen(argv[i+2],"rb")) ) { perror(argv[i+1]); exit(0); }
  }
  if(!freopen(argv[input],"rb",stdin)) { perror(argv[input]); exit(0); }

  /* copy input data to memory */
  for(i=0;i<ref_n;++i){
    if(dread_file(fp_ref[i],&ref_buf[i]) < 0) {perror(argv[i]); exit(0);}
    ref[i] = ref_buf[i].buffer; 
    ref_s[i] = ref_buf[i].pos - ref_buf[i].buffer; 
  }
  if(dread_file(stdin,&delta) < 0) perror(argv[input]);

  /* decompress the data */
  if(zd_uncompressN1((const Bytef**)ref, ref_s, ref_n,
		    &tar_buf, &t_size,
		    (const Bytef*) delta.buffer, delta.pos - delta.buffer) == ZD_OK){
    
    /* successfull compression write the delta to a file */
    if(argc>4 && (!freopen(argv[output],"wb",stdout))){
      perror(argv[output]);
      exit(0);
    }
    
    if(t_size !=  fwrite(tar_buf, sizeof(char),t_size ,stdout))
      perror("ouput");

    free(tar_buf);
  }
  
  /* release memory */
  zd_free(&delta);
  for(i=ref_n-1;i>=0;--i){
    zd_free(&ref_buf[i]); /* free  ref. buffers */
    fclose(fp_ref[i]);   /* close ref files */ 
  }

  return 0;
}
#endif /* REFNUM>1 */

/*===========================================================================
 *  single reference files
 */
int zd_patch_single(int argc, char *argv[]){
  FILE *fp_ref;

  zd_mem_buffer ref_buf;
  zd_mem_buffer delta;
  Bytef *tar_buf = NULL;
  uLong t_size = 0;

  /* open and check the input files */
  if( !(fp_ref = fopen(argv[2],"rb")) )
  {
    perror(argv[2]);
    exit(0);
  }

  if(argc>3 && (!freopen(argv[3],"rb",stdin)))
  {
    perror(argv[3]); 
    exit(0);
  }

  /* copy input data to memory */
  if(dread_file(fp_ref,&ref_buf) < 0) {perror(argv[1]); exit(0);}
  if(dread_file(stdin,&delta) < 0) {perror(argv[2]); exit(0);}

  /* decompress the data */
  if(zd_uncompress1((const Bytef*)ref_buf.buffer, ref_buf.pos - ref_buf.buffer,
		    &tar_buf, &t_size,
		    (const Bytef*) delta.buffer, 
		    delta.pos - delta.buffer) == ZD_OK){
    
    /* successfull compression write the delta to a file */
    if(argc>4 && (!freopen(argv[4],"wb",stdout))){
      perror(argv[4]);
      exit(0);
    }
    if(t_size !=  fwrite(tar_buf, sizeof(char),t_size ,stdout))
      perror("ouput");
  }
  
  /* release memory */
  free(tar_buf);
  zd_free(&delta);
  zd_free(&ref_buf);
  
  /* close files */
  fclose(fp_ref);

  return 0;
}

/*===========================================================================
 *  main function
 */

int main(int argc, char *argv[]){
  int i;

  if(argc<3 || argc>(REFNUM+4))   /* check command line parameters */
  {
    fprintf(stderr,"%s\n",usage_single);
    fprintf(stderr,"%s\n",usage_multi);
    exit(0);
  }
  
  if (strcmp("diff", argv[1]) == 0)
  {
	  if(argc <= 5){
		return zd_diff_single(argc,argv);
	  }
	  else{
		return ZDDiffMulti(argc, argv);
	  }
  }
  else if (strcmp("patch", argv[1]) == 0)
  {
	  if(argc <= 5){
		return zd_patch_single(argc,argv);
	  }
	  else{
		return ZDPatchMulti(argc, argv);
	  }
  } else {
    fprintf(stderr,"%s\n",usage_single);
    fprintf(stderr,"%s\n",usage_multi);
    exit(0);
  }
}

