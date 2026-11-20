# flex's C++ skeleton (2.6.4, 2.6.15, 2.7.1) loses memory when an allocation fails:
# yy_create_buffer() keeps the buffer struct when its character buffer cannot
# be allocated, and yy_get_next_buffer(), yyensure_buffer_stack() and
# yy_push_state() assign the result of realloc() over the only pointer to the
# old block. The scanners' LexerError() throws std::bad_alloc, so the scanner
# lives on and the lost blocks leak. This script, run on each generated
# scanner, frees the struct and keeps the old blocks until realloc()
# succeeds; it fails if the skeleton no longer has the code it patches.
FILE(READ "${SCANNER}" TEXT)

FUNCTION(PATCH WHERE OLD NEW)
  STRING(FIND "${TEXT}" "${OLD}" POSITION)
  IF (POSITION EQUAL -1)
    MESSAGE(FATAL_ERROR "${SCANNER}: ${WHERE}: the code to patch is not there; check the flex skeleton")
  ENDIF ()
  STRING(REPLACE "${OLD}" "${NEW}" TEXT "${TEXT}")
  SET(TEXT "${TEXT}" PARENT_SCOPE)
ENDFUNCTION()

PATCH("yy_create_buffer"
[==[	b->yy_ch_buf = (char *) yyalloc( (yy_size_t) (b->yy_buf_size + 2)  );
	if ( b->yy_ch_buf == NULL ) {
		YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );]==]
[==[	b->yy_ch_buf = (char *) yyalloc( (yy_size_t) (b->yy_buf_size + 2)  );
	if ( b->yy_ch_buf == NULL ) {
		yyfree( (void *) b );
		YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );]==])

PATCH("yy_get_next_buffer (input buffer overflow)"
[==[				if ( new_size <= 0 ) {
					b->yy_buf_size += b->yy_buf_size / 8;
				} else {
					b->yy_buf_size *= 2;
				}
				b->yy_ch_buf = (char *)
					/* Include room in for 2 EOB chars. */
					yyrealloc( (void *) b->yy_ch_buf,
							 (yy_size_t) (b->yy_buf_size + 2)  );]==]
[==[				int yy_old_buf_size = b->yy_buf_size;
				char *yy_new_ch_buf;
				if ( new_size <= 0 ) {
					b->yy_buf_size += b->yy_buf_size / 8;
				} else {
					b->yy_buf_size *= 2;
				}
				yy_new_ch_buf = (char *)
					/* Include room in for 2 EOB chars. */
					yyrealloc( (void *) b->yy_ch_buf,
							 (yy_size_t) (b->yy_buf_size + 2)  );
				if ( ! yy_new_ch_buf ) {
					b->yy_buf_size = yy_old_buf_size;
					YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
				}
				b->yy_ch_buf = yy_new_ch_buf;]==])

PATCH("yy_get_next_buffer (extension)"
[==[		YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = (char *) yyrealloc(
			(void *) YY_CURRENT_BUFFER_LVALUE->yy_ch_buf, (yy_size_t) new_size  );
		if ( ! YY_CURRENT_BUFFER_LVALUE->yy_ch_buf ) {
			YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
		}]==]
[==[		char *yy_new_ch_buf = (char *) yyrealloc(
			(void *) YY_CURRENT_BUFFER_LVALUE->yy_ch_buf, (yy_size_t) new_size  );
		if ( ! yy_new_ch_buf ) {
			YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
		}
		YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = yy_new_ch_buf;]==])

PATCH("yyensure_buffer_stack"
[==[		num_to_alloc = (yy_buffer_stack_max) + grow_size;
		(yy_buffer_stack) = (struct yy_buffer_state**)yyrealloc
								((yy_buffer_stack),
								num_to_alloc * sizeof(struct yy_buffer_state*)
								);
		if ((yy_buffer_stack) == NULL) {
			YY_FATAL_ERROR( "out of dynamic memory in yyensure_buffer_stack()" );
		}]==]
[==[		struct yy_buffer_state** yy_new_buffer_stack;
		num_to_alloc = (yy_buffer_stack_max) + grow_size;
		yy_new_buffer_stack = (struct yy_buffer_state**)yyrealloc
								((yy_buffer_stack),
								num_to_alloc * sizeof(struct yy_buffer_state*)
								);
		if (yy_new_buffer_stack == NULL) {
			YY_FATAL_ERROR( "out of dynamic memory in yyensure_buffer_stack()" );
		}
		(yy_buffer_stack) = yy_new_buffer_stack;]==])

PATCH("yy_push_state"
[==[		yy_size_t new_size;

		(yy_start_stack_depth) += YY_START_STACK_INCR;
		new_size = (yy_size_t) (yy_start_stack_depth) * sizeof( int );

		if ( ! (yy_start_stack) ) {
			(yy_start_stack) = (int *) yyalloc( new_size  );

		} else {
			(yy_start_stack) = (int *) yyrealloc(
					(void *) (yy_start_stack), new_size  );
		}
		if ( ! (yy_start_stack) ) {
			YY_FATAL_ERROR( "out of memory expanding start-condition stack" );
		}]==]
[==[		yy_size_t new_size;
		int *yy_new_start_stack;

		new_size = (yy_size_t) ((yy_start_stack_depth) + YY_START_STACK_INCR) * sizeof( int );

		if ( ! (yy_start_stack) ) {
			yy_new_start_stack = (int *) yyalloc( new_size  );

		} else {
			yy_new_start_stack = (int *) yyrealloc(
					(void *) (yy_start_stack), new_size  );
		}
		if ( ! yy_new_start_stack ) {
			YY_FATAL_ERROR( "out of memory expanding start-condition stack" );
		}
		(yy_start_stack) = yy_new_start_stack;
		(yy_start_stack_depth) += YY_START_STACK_INCR;]==])

FILE(WRITE "${SCANNER}" "${TEXT}")
