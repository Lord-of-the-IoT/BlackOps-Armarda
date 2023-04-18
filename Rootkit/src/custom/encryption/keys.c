//this file is a mess and doesnt really work, so ignore this!!!


/*
https://www.youtube.com/watch?v=k-Y9vEqB-xQ
*/
#include <linux/math.h>
#include <linux/stdarg.h>
#include <linux/string.h>


static struct key_t{ //key structure
  uint64_t key[8]; //512 bits
  /*
     7     6     5    4      3     2     1     0
  |XXXXX|XXXXX|XXXXX|XXXXX|XXXXX|XXXXX|XXXXX|XXXXX|
  */
  char hex[129]; //128 charecters and termination charecter
};

static char hex_vals[16]="0123456789ABCDEF"; //contains all of the hex values
static uint64_t null_key[8]= {(uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0}; //null key for generate_key function

struct key_t generate_key(uint64_t number[8]);
int free_key(struct key_t *key);
int update_hex(struct key_t *key);
int rand_key(struct key_t *key);
struct key_t multiply_keys(struct key_t key1, struct key_t key2);
struct key_t add_keys(struct key_t key1, struct key_t key2);
struct key_t rshift_key(struct key_t key, int shift);
struct key_t lshift_key(struct key_t key, int shift);
int cmp_keys(struct key_t key1, struct key_t key2);
struct key_t pow_keys(struct key_t base, struct key_t exponent);
struct key_t key_from_hex(char *hex);

struct key_t generate_key(uint64_t number[8]){ // creates key
  struct key_t return_obj; //creates object tto be returned
  for(int i=0; i<8; i++){ //loops through values passed
    return_obj.key[i] = number[i]; //sets the return_obj to corresponding number
  }
  update_hex(&return_obj);
  return return_obj; //returns number
};

int free_key(struct key_t *key){
  kfree(key);
  return 0;
};


int update_hex(struct key_t *key){ //returns hex string representation of key
  for (int i=0; i<128; i++){
    key->hex[i] = hex_vals[key->key[i/16]<<i%16*4>>60];
  }
  return 0;
};

int rand_key(struct key_t *key){
  for (int i=0; i<8; i++){
    get_random_bytes(&(key->key[i]), 8);
  }
  return 0;
};

//operators
struct key_t multiply_keys(struct key_t key1, struct key_t key2){
  struct key_t product_key = generate_key(null_key);
  uint64_t overflow[8] = {(uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0};
  uint64_t temp=0;
  //multiplies and stores overflow
  for (int i=0; i<8; i++){
    for (int j=0; j<8; j++){
      product_key.key[i] += key1.key[i]*key2.key[j];
      overflow[i] = __builtin_umulll_overflow(key1.key[i], key2.key[j], &(product_key.key[i]));
    }
  }
  //adds all overflow on
  while(overflow[1]!=(uint64_t) 0 || overflow[2]!=(uint64_t) 0 || overflow[3]!=(uint64_t) 0 ||
        overflow[4]!=(uint64_t) 0 || overflow[5]!=(uint64_t) 0 || overflow[6]!=(uint64_t) 0){ //while overflow
    for (int i=1; i<8; i++){
      temp = product_key.key[i];
      product_key.key[i] += overflow[i-1];
      overflow[i] = __builtin_uaddll_overflow(overflow[i-1], temp, &(product_key.key[i]));
    }
  }
  return product_key;
};

struct key_t add_keys(struct key_t key1, struct key_t key2){
  struct key_t product_key = generate_key(null_key);
  uint64_t overflow[8] = {0};
  uint64_t temp=0;

  //adds and stores overflow
  for (int i=0; i<8; i++){
    product_key.key[i] = key1.key[i]+key2.key[i];
    overflow[i] = __builtin_uaddll_overflow(key1.key[i], key2.key[i], &(product_key.key[i]));
  }

  //adds all overflow on
  while(!(overflow[0]==0&&overflow[1]==0&&overflow[2]==0&&overflow[3]==0&&overflow[4]==0&&overflow[5]==0&&overflow[6]==0)){ //while overflow
    for (int i=1; i<8; i++){
      temp = product_key.key[i];
      product_key.key[i] += overflow[i-1];
      overflow[i] = __builtin_uaddll_overflow(overflow[i], temp, &(product_key.key[i]));
    }
  }
  return product_key;

}

struct key_t rshift_key(struct key_t key, int shift){
  struct key_t product_key = generate_key(null_key);
  uint64_t overflow=0;
  if (shift>64){
    shift = 64;
  }
  if (shift=64){
    for(int i=0; i<7; i++){
      product_key.key[i] = key.key[i+1];
    }
    product_key.key[7] = (uint64_t) 0;
  }
  else{
    for(int i=7; i>0; i--){
      product_key.key[i]= key.key[i]>>shift+overflow; //shifts and adds overflow on- overflow is same size as the 0 bits
      overflow = key.key[i]<<(64-shift);
    }
  }
  return product_key;
}
struct key_t lshift_key(struct key_t key, int shift){
  struct key_t product_key = generate_key(null_key);
  uint64_t overflow=0;
  if (shift>64){
    shift = 64;
  }
  if (shift=64){
    for(int i=7; i<1; i++){
      product_key.key[i] = key.key[i-1];
    }
    product_key.key[0] = (uint64_t) 0;
  }
  else{
    for(int i=0; i<7; i--){
      product_key.key[i]= key.key[i]<<shift+overflow; //shifts and adds overflow on- overflow is same size as the 0 bits
      overflow = key.key[i]>>(64-shift);
    }
  }
  return product_key;
}
int cmp_keys(struct key_t key1, struct key_t key2){
  //returns 1 if first number bigger, returns -1 if second number bigger, returns 0 if equal
  for(int i =7; i>0; i--){
    if (key1.key[i]>key2.key[i]){
      return 1;
    }
    if (key1.key[i]<key2.key[i]){
        return -1;
    }
  }
  return 0;
}

struct key_t pow_keys(struct key_t base, struct key_t exponent){
  uint64_t product_key_val[8] = {(uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 1};
  struct key_t product_key = generate_key(product_key_val);
  struct key_t empty_key = generate_key(null_key);//for comparison between exponent
  while (cmp_keys(exponent, empty_key)>0){ //while exponent>0
    if (exponent.key[0]&1){ //if exponent % 2 == 1
        product_key = multiply_keys(product_key, base);
        update_hex(&product_key);
    }
    exponent = rshift_key(exponent, 1); //halfs exponent
    update_hex(&exponent);

    base = multiply_keys(base, base); //squares base
    update_hex(&base);
  }
  return product_key;
}

struct key_t key_from_hex(char *hex){
  struct key_t return_obj = generate_key(null_key);
  for (int i=0; i<128; i++){
    return_obj.key[i/8] ^= ((uint64_t) hex[i]) << i%8*8;
  }
  return return_obj;
}
