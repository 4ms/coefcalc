#coefcalc
##Coefficient calculator for Spectral Multiband Resonator
Author: Dan Green danngreen1@gmail.com

How to Use:

  1. Change the parameters in `main.c`:
  
      - `scale_type`
  
      - `sample_rate`
  
      - and possibly `start_freq` inside the `if(scale_type==XXXX)` block
  
      - And/or create your own `scale_type`

  2. `./mk && ./calc-filters` 
  
  4. Pipe or paste the output into a C header file, and put it in the ../SMR/filter_coefs/ directory
  
  5. Modify SMR code to use the new filter coefs, by `#include`ing it in ../SMR/filter_coef.h and adding it to the list in ../SMR/filter.c, and adding an entry in the LED ring colors


This is just a front-end to fidlib, please see README-fidlib