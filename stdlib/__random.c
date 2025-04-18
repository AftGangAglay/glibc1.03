/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This is derived from the Berkeley source:
 *	@(#)random.c	5.5 (Berkeley) 7/6/88
 * It was reworked for the GNU C Library by Roland McGrath.
 */

/* An improved random number generation package.  In addition to the standard
   rand()/srand() like interface, this package also has a special
   glibc_rand_state info interface.
   The initstate() routine is called with a seed, an array of
   bytes, and a count of how many bytes are being passed in; this array is
   then initialized to contain information for random number generation with
   that much glibc_rand_state information.  Good sizes for the amount of
   glibc_rand_state information are 32, 64, 128, and 256 bytes.
   The glibc_rand_state can be switched by calling the setstate() function with
   the same array as was initiallized with initstate().
   By default, the package runs with 128 bytes of glibc_rand_state
   information and generates far better random numbers than a linear
   congruential generator.  If the amount of glibc_rand_state information is less than
   32 bytes, a simple linear congruential R.N.G. is used.  Internally, the
   glibc_rand_state information is treated as an array of asys_native_long_ts;
   the zeroeth element of the array is the type of R.N.G. being used
   (small integer); the remainder of the array is the glibc_rand_state
   information for the R.N.G. Thus, 32 bytes of glibc_rand_state information
   will give 7 asys_native_long_ts worth of glibc_rand_state information, which
   will allow a degree seven polynomial. (Note: The zeroeth word of
   glibc_rand_state information also has some other information stored in it;
   see setstate for details).
   The random number generation technique is a linear feedback shift
   approach, employing trinomials (since there are fewer terms
   to sum up that way).  In this approach, the least significant bit of all
   the numbers in the glibc_rand_state table will act as a linear feedback
   shift register, and will have period 2^deg - 1
   (where deg is the degree of the polynomial being used, assuming that the
   polynomial is irreducible and primitive). The higher order bits will have
   asys_native_long_ter periods, since their values are also influenced by
   pseudo-random carries out of the lower bits. The total period of the
   generator is approximately deg*(2**deg - 1); thus doubling the amount of
   glibc_rand_state information has a vast influence on the period of the
   generator.  Note: The deg*(2**deg - 1) is an approximation
   only good for large deg, when the period of the shift is the
   dominant factor.  With deg equal to seven, the period is actually much
   asys_native_long_ter than the 7*(2**7 - 1) predicted by this formula.  */



/* For each of the currently supported random number generators, we have a
   break value on the amount of glibc_rand_state information (you need at least
   thi bytes of glibc_rand_state info to support this random number generator),
   a degree for the polynomial (actually a trinomial) that the R.N.G. is based
   on, and separation between the two lower order coefficients of the
   trinomial.  */

/* Linear congruential.  */
#define	GLIBC_RAND_TYPE_0 (0)
#define	GLIBC_RAND_BREAK_0 (8)
#define	GLIBC_RAND_DEG_0 (0)
#define	GLIBC_RAND_SEP_0 (0)

/* x**7 + x**3 + 1.  */
#define	GLIBC_RAND_TYPE_1 (1)
#define	GLIBC_RAND_BREAK_1 (32)
#define	GLIBC_RAND_DEG_1 (7)
#define	GLIBC_RAND_SEP_1 (3)

/* x**15 + x + 1.  */
#define	GLIBC_RAND_TYPE_2 (2)
#define	GLIBC_RAND_BREAK_2 (64)
#define	GLIBC_RAND_DEG_2 (15)
#define	GLIBC_RAND_SEP_2 (1)

/* x**31 + x**3 + 1.  */
#define	GLIBC_RAND_TYPE_3 (3)
#define	GLIBC_RAND_BREAK_3 (128)
#define	GLIBC_RAND_DEG_3 (31)
#define	GLIBC_RAND_SEP_3 (3)

/* x**63 + x + 1.  */
#define	GLIBC_RAND_TYPE_4 (4)
#define	GLIBC_RAND_BREAK_4 (256)
#define	GLIBC_RAND_DEG_4 (63)
#define	GLIBC_RAND_SEP_4 (1)


/* Array versions of the above information to make code run faster.
   Relies on fact that GLIBC_RAND_TYPE_i == i.  */

#define	GLIBC_MAX_TYPES (5) /* Max number of types above. */

static const asys_native_long_t glibc_rand_degrees[GLIBC_MAX_TYPES] = {
		GLIBC_RAND_DEG_0,
		GLIBC_RAND_DEG_1,
		GLIBC_RAND_DEG_2,
		GLIBC_RAND_DEG_3,
		GLIBC_RAND_DEG_4
};

static const asys_native_long_t glibc_rand_seps[GLIBC_MAX_TYPES] = {
		GLIBC_RAND_SEP_0,
		GLIBC_RAND_SEP_1,
		GLIBC_RAND_SEP_2,
		GLIBC_RAND_SEP_3,
		GLIBC_RAND_SEP_4
};



/* Initially, everything is set up as if from:
	initstate(1, glibc_rand_table, 128);
   Note that this initialization takes advantage of the fact that srandom
   advances the front and rear pointers 10*glibc_rand_deg times, and hence the
   rear pointer which starts at 0 will also end up at zero; thus the zeroeth
   element of the glibc_rand_state information, which contains info about the
   current position of the rear pointer is just
	(GLIBC_MAX_TYPES * (glibc_rand_back - glibc_rand_state)) +
 		GLIBC_RAND_TYPE_3 == GLIBC_RAND_TYPE_3
 */

static asys_native_long_t glibc_rand_table[GLIBC_RAND_DEG_3 + 1] = {
		GLIBC_RAND_TYPE_3,
		0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
		0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
		0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
		0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
		0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
		0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
		0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
		0xf5ad9d0e, 0x8999220b, 0x27fb47b9
};

/* FPTR and RPTR are two pointers into the glibc_rand_state info, a front and a
   rear pointer.  These two pointers are always glibc_rand_sep places aparts,
   as they cycle through the glibc_rand_state information.  (Yes, this does
   mean we could get away with just one pointer, but the code for random is
   more efficient this way).  The pointers are left positioned as they would be
   from the call: initstate(1, glibc_rand_table, 128);
   (The position of the rear pointer, glibc_rand_back, is really 0 (as
   explained above in the initialization of glibc_rand_table) because the
   glibc_rand_state table pointer is set to point to glibc_rand_table[1]
   (as explained below).)  */

static asys_native_long_t* glibc_rand_front =
		&glibc_rand_table[GLIBC_RAND_SEP_3 + 1];

static asys_native_long_t* glibc_rand_back = &glibc_rand_table[1];

/* The following things are the pointer to the glibc_rand_state information
   table, the type of the current generator, the degree of the current
   polynomial being used, and the separation between the two pointers.
   Note that for efficiency of random, we remember the first location of
   the glibc_rand_state information, not the zeroeth.  Hence it is valid to
   access glibc_rand_state[-1], which is used to store the type of the R.N.G.
   Also, we remember the last location, since this is more efficient than
   indexing every time to find the address of the last element to see if
   the front and rear pointers have wrapped.  */

static asys_native_long_t* glibc_rand_state = &glibc_rand_table[1];

static asys_native_long_t glibc_rand_type = GLIBC_RAND_TYPE_3;
static asys_native_long_t glibc_rand_deg = GLIBC_RAND_DEG_3;
static asys_native_long_t glibc_rand_sep = GLIBC_RAND_SEP_3;

static const asys_native_long_t* glibc_rand_end =
		&glibc_rand_table[sizeof(glibc_rand_table) / sizeof(glibc_rand_table[0])];

/* Initialize the random number generator based on the given seed.  If the
   type is the trivial no-glibc_rand_state-information type, just remember the
   seed. Otherwise, initializes glibc_rand_state[] based on the given "seed"
   via a linear congruential generator.  Then, the pointers are set to known
   locations that are exactly glibc_rand_sep places apart.  Lastly, it cycles
   the glibc_rand_state information a given number of times to get rid of any
   initial dependencies introduced by the L.C.R.N.G.  Note that the
   initialization of glibc_rand_table[] for default usage relies on values
   produced by this routine.  */

asys_native_long_t glibc___random(void);

void glibc___srandom(unsigned x) {
	glibc_rand_state[0] = x;

	if(glibc_rand_type != GLIBC_RAND_TYPE_0) {
		asys_native_long_t i;
		for(i = 1; i < glibc_rand_deg; ++i) {
			glibc_rand_state[i] =
					(1103515145 * glibc_rand_state[i - 1]) + 12345;
		}

		glibc_rand_front = &glibc_rand_state[glibc_rand_sep];
		glibc_rand_back = &glibc_rand_state[0];

		for(i = 0; i < 10 * glibc_rand_deg; ++i) glibc___random();
	}
}

/* Initialize the glibc_rand_state information in the given array of N bytes
   for future random number generation.  Based on the number of bytes we
   are given, and the break values for the different R.N.G.'s, we choose
   the best (largest) one we can and set things up for it.  srandom is
   then called to initialize the glibc_rand_state information.  Note that on
   return from srandom, we set glibc_rand_state[-1] to be the type multiplexed
   with the current value of the rear pointer; this is so successive calls to
   initstate won't lose this information and will be able to restart with
   setstate. Note: The first thing we do is save the current glibc_rand_state,
   if any, just like setstate so that it doesn't matter when initstate is
   called. Returns a pointer to the old glibc_rand_state.  */
void* glibc___initstate(unsigned seed, void* arg_state, asys_size_t n) {
	void* ostate = (void*) &glibc_rand_state[-1];

	if(glibc_rand_type == GLIBC_RAND_TYPE_0) {
		glibc_rand_state[-1] = glibc_rand_type;
	}
	else {
		glibc_rand_state[-1] =
				(GLIBC_MAX_TYPES * (glibc_rand_back - glibc_rand_state)) +
				glibc_rand_type;
	}

	if(n < GLIBC_RAND_BREAK_1) {
		if(n < GLIBC_RAND_BREAK_0) return 0;

		glibc_rand_type = GLIBC_RAND_TYPE_0;
		glibc_rand_deg = GLIBC_RAND_DEG_0;
		glibc_rand_sep = GLIBC_RAND_SEP_0;
	}
	else if(n < GLIBC_RAND_BREAK_2) {
		glibc_rand_type = GLIBC_RAND_TYPE_1;
		glibc_rand_deg = GLIBC_RAND_DEG_1;
		glibc_rand_sep = GLIBC_RAND_SEP_1;
	}
	else if(n < GLIBC_RAND_BREAK_3) {
		glibc_rand_type = GLIBC_RAND_TYPE_2;
		glibc_rand_deg = GLIBC_RAND_DEG_2;
		glibc_rand_sep = GLIBC_RAND_SEP_2;
	}
	else if(n < GLIBC_RAND_BREAK_4) {
		glibc_rand_type = GLIBC_RAND_TYPE_3;
		glibc_rand_deg = GLIBC_RAND_DEG_3;
		glibc_rand_sep = GLIBC_RAND_SEP_3;
	}
	else {
		glibc_rand_type = GLIBC_RAND_TYPE_4;
		glibc_rand_deg = GLIBC_RAND_DEG_4;
		glibc_rand_sep = GLIBC_RAND_SEP_4;
	}

	/* First location. */
	glibc_rand_state = &((asys_native_long_t*) arg_state)[1];

	/* Must set END_PTR before srandom. */
	glibc_rand_end = &glibc_rand_state[glibc_rand_deg];

	glibc___srandom(seed);

	if(glibc_rand_type == GLIBC_RAND_TYPE_0) {
		glibc_rand_state[-1] = glibc_rand_type;
	}
	else {
		glibc_rand_state[-1] =
				(GLIBC_MAX_TYPES * (glibc_rand_back - glibc_rand_state)) +
				glibc_rand_type;
	}

	return ostate;
}

/* Restore the glibc_rand_state from the given glibc_rand_state array.
   Note: It is important that we also remember the locations of the pointers
   in the current glibc_rand_state information, and restore the locations of
   the pointers from the old glibc_rand_state information.
   This is done by multiplexing the pointer location into the zeroeth word of
   the glibc_rand_state information. Note that due to the order in which things
   are done, it is OK to call setstate with the same glibc_rand_state as the
   current glibc_rand_state Returns a pointer to the old glibc_rand_state
   information.  */
void* glibc___setstate(void* arg_state) {
	asys_native_long_t* new_state = arg_state;
	asys_native_long_t type = new_state[0] % GLIBC_MAX_TYPES;
	asys_native_long_t rear = new_state[0] / GLIBC_MAX_TYPES;

	void* ostate = &glibc_rand_state[-1];

	if(glibc_rand_type == GLIBC_RAND_TYPE_0) {
		glibc_rand_state[-1] = glibc_rand_type;
	}
	else {
		glibc_rand_state[-1] =
				(GLIBC_MAX_TYPES * (glibc_rand_back - glibc_rand_state)) +
				glibc_rand_type;
	}

	switch (type) {
		case GLIBC_RAND_TYPE_0:; ASYS_FALLTHROUGH;
		/* FALLTHROUGH */
		case GLIBC_RAND_TYPE_1:; ASYS_FALLTHROUGH;
		/* FALLTHROUGH */
		case GLIBC_RAND_TYPE_2:; ASYS_FALLTHROUGH;
		/* FALLTHROUGH */
		case GLIBC_RAND_TYPE_3:; ASYS_FALLTHROUGH;
		/* FALLTHROUGH */
		case GLIBC_RAND_TYPE_4: {
			glibc_rand_type = type;
			glibc_rand_deg = glibc_rand_degrees[type];
			glibc_rand_sep = glibc_rand_seps[type];
			break;
		}

		default: return 0;
	}

	glibc_rand_state = &new_state[1];

	if(glibc_rand_type != GLIBC_RAND_TYPE_0) {
		glibc_rand_back = &glibc_rand_state[rear];
		glibc_rand_front =
				&glibc_rand_state[(rear + glibc_rand_sep) % glibc_rand_deg];
	}

	/* Set glibc_rand_end too.  */
	glibc_rand_end = &glibc_rand_state[glibc_rand_deg];

	return ostate;
}

/* If we are using the trivial GLIBC_RAND_TYPE_0 R.N.G., just do the old linear
   congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
   same in all ther other cases due to all the global variables that have been
   set up.  The basic operation is to add the number at the rear pointer into
   the one at the front pointer.  Then both pointers are advanced to the next
   location cyclically in the table.  The value returned is the sum generated,
   reduced to 31 bits by throwing away the "least random" low bit.
   Note: The code takes advantage of the fact that both the front and
   rear pointers can't wrap on the same call by not testing the rear
   pointer if the front one has wrapped.  Returns a 31-bit random number.  */

asys_native_long_t glibc___random(void) {
	if(glibc_rand_type == GLIBC_RAND_TYPE_0) {
		glibc_rand_state[0] =
				((glibc_rand_state[0] * 1103515245) + 12345) &
				ASYS_NATIVE_LONG_MAX;

		return glibc_rand_state[0];
	}
	else {
		asys_native_long_t i;
		*glibc_rand_front += *glibc_rand_back;

		/* Chucking least random bit.  */
		i = (*glibc_rand_front >> 1) & ASYS_NATIVE_LONG_MAX;
		++glibc_rand_front;

		if(glibc_rand_front >= glibc_rand_end) {
			glibc_rand_front = glibc_rand_state;
			++glibc_rand_back;
		}
		else {
			++glibc_rand_back;

			if(glibc_rand_back >= glibc_rand_end) {
				glibc_rand_back = glibc_rand_state;
			}
		}

		return i;
	}
}
