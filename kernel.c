struct word{
    uchar b0;
    uchar b1;
    uchar b2;
    uchar b3;
};

__constant uchar sbox[] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
    };

struct word xor(const struct word w1, const struct word w2){
    return (struct word){(w1.b0^w2.b0), (w1.b1^w2.b1), (w1.b2^w2.b2), (w1.b3^w2.b3)};
}

struct word sub_word(const struct word w){
    return (struct word){sbox[w.b0], sbox[w.b1], sbox[w.b2], sbox[w.b3]};
}

struct word rot_word(const struct word w){
    return (struct word){w.b1, w.b2, w.b3, w.b0};
}

__constant uchar rcon_table[] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a};

struct word get_rcon_for(const int idx){
    return (struct word){rcon_table[idx], 0x00, 0x00, 0x00};
}

__kernel void calc_schedules(
    __global const uchar *harr_g, __global uchar *res_g, const int keylen)
{
  //Set parameters according to key length
  const int Nb = 4;
  const int Nk = keylen/32;
  const int Nr = Nk+6;

  //Key schedule size, given as number of words (with each word taking 4 bytes)
  const int ks_size = (4*(Nr+1));
  
  //Get 128 or 258 bit potential key from current offs
  int curr_offs = get_global_id(0);

  unsigned char pot_key[32];
  int i;
  for(i=0; i<(keylen/8); ++i){
      pot_key[i] = harr_g[curr_offs + i];
  }

  //Calculate key_schedule from this potential key
  //Schedule is given words
  struct word pot_ks[60];
  //Init ks first..
  for(i=0; i<Nk; ++i){
      pot_ks[i] = (struct word){pot_key[4*i], pot_key[4*i+1], pot_key[4*i+2], pot_key[4*i+3]};
  }

  //Now calculate rest of the potential ks
  for(i=Nk; i<(Nb*(Nr+1)); ++i){
      struct word tmpw = pot_ks[i-1];

      if((i % Nk) == 0){
           tmpw = xor(sub_word(rot_word(tmpw)), get_rcon_for(i/Nk));
      }
      else if( (Nk > 6)  && (i%Nk == 4)){ 
          tmpw = sub_word(tmpw);
      }

      pot_ks[i] = xor(pot_ks[i-Nk], tmpw);
  }

  //Now check if calculated ks matches pattern given at the current offset
  //Set current offset to 1 if thats the case, 0 otherwise
  res_g[curr_offs] = 1;
  for(i=0; i<ks_size; ++i){
        if(
            (pot_ks[i].b0 != harr_g[curr_offs+i*4]) ||
            (pot_ks[i].b1 != harr_g[curr_offs+i*4+1]) ||
            (pot_ks[i].b2 != harr_g[curr_offs+i*4+2]) ||
            (pot_ks[i].b3 != harr_g[curr_offs+i*4+3])
        ){
             res_g[curr_offs] = 0; 
             break;
        }
  }

  //If there is a key at the current offset, print it
  if(res_g[curr_offs] == 1){
      printf("Found key at offset 0x%x\n", curr_offs);
  }
}