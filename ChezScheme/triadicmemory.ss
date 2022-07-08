#! /usr/local/bin/scheme --script  # triadicmemory.ss Copyright (c) 2022 Roger Turner

#| Based on TriadicMemory/C/triadicmemory.c by Peter Overmann

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
  and associated documentation files (the “Software”), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial
  portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
  LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This Chez Scheme library can be used to create and query triadic-memory records
to store triples {x,y,z} of sparse distributed representations (SDRs), and to
recall one part of a triple by specifying the other two parts.
{x,y,_} recalls the third part, {x,_,z} recalls the second part, {_,y,z} recalls
the first part.

SDRs are represented as sorted lists of p Fixnums in the range 0 to n-1.
Typical values are n = 1000 and p = 10
For each triple, counts of all p*p*p combinations are added to a n*n*n cube of bytes.
In the default nybble mode both x*y*z counts and z*y*x counts are stored in each byte.

Based on the following version of triadicmemory.c:
https://github.com/PeterOvermann/TriadicMemory/blob/676233aaeeff9579da243de4b6164de2872bfbdc/triadicmemory.c
For convenience of comparison, code layout and many identifiers echo that C code.

See triadic-test.ss for example of API usage.

If the import/main calls at the end are uncommented this file is executable*
(with Chez Scheme installed) eg:

  $ chmod +x triadicmemory.ss
  $ triadicmemory.ss 9 3
  {1 2 3, 4 5 6, 7 8 9}
  {1 2 3, 4 5 6, _}
  (7 8 9)
  quit
  $
  
(* Note that this is incompatible with use as a library, eg by triadic-test.ss *)

|#
  
#!chezscheme

(library (triadicmemory) (export
make-triadic-memory
store
delete
query
main)

(import (chezscheme))

(define-record-type triadic-memory
  (fields
    (mutable   N    tm-N tm-N!)          ;; positive for nybble mode
    (immutable P    tm-P)
    (immutable cube tm-cube))
  (sealed #t) (opaque #t) (nongenerative triadic-memory)
(protocol #;(make-triadic-memory N P)    ;; Nat Nat -> TM
  ;; produce a triadic-memory record with given N, P (N forced to multiple of 4)
  (lambda (new)
    (lambda (N P)
      (let ([N (fx* 4 (fxdiv (fx+ 3 N) 4))])
        (new N P (make-bytevector (fx* N N N) 0)))))))
                                                                                            ;
(define (parse tm cs)                    ;; TM ListOfChar -> (ListOf 0..N-1) ListOfChar
  ;; produce list (length P or null), rest of cs after "," or "}"
  ;; example: "37 195 ...}" => (37 195 ...) ""
  ;;          "_ , 73 ...}" => ()           " 73 ..."
  (define (char->digit c) ;; Char -> 0-9
    (fx- (char->integer c) (char->integer #\0)))
  (let next-c ([cs cs] [vs (list)])
    (if (pair? cs)
      (let ([c (car cs)])
        (cond
          [(char-whitespace? c)  (next-c (cdr cs) vs) ]
          [(char=? c #\_)        (next-c (cdr cs) (list)) ]
          [(char-numeric? c)
            (let next-d ([cs (cdr cs)] [v (char->digit c)])
              (cond
                [(null? cs)  (error 'parse "missing terminator") ]
                [(char-numeric? (car cs))
                  (next-d (cdr cs) (fx+ (fx* 10 v) (char->digit (car cs)))) ]
                [else (if (fx<=? 1 v (tm-N tm))
                        (next-c cs (cons (fx1- v) vs))
                        (error 'parse "SDR element not 1-N")) ])) ]
          [(or (char=? c #\,) (char=? c #\}))
            (if (or (null? vs)
                    (and (apply fx>? vs) (fx=? (length vs) (tm-P tm))))
              (values (reverse! vs) (cdr cs))
              (error 'parse "SDR wrong length or not sorted")) ]
          [else (error 'parse "invalid character") ]))
      (error 'parse "missing terminator"))))
                                                                                            ;
(define (binarize v n p)                 ;; (FXVector N->Fixnum) N P -> ListOfFixnum
  ;; produce indices of p-highest elements of v
  (let* (
      [maxp (list-sort fx<?              ;; first p elements sorted
              (do ( [i 0 (fx1+ i)]
                    [m (list) (cons (fxvector-ref v i) m)])
                  ((fx=? i p) m))) ]
      ;; rankedmax computation is equivalent to:
          #;[sorted    (vector-sort fx<? v)]
          #;[rankedmax (vector-ref sorted (fx- n p))]
      [rankedmax
        (let next-v ([i p] [maxp maxp])  ;; maxp is p highest elements
          (if (fx=? i n)
            (car maxp)
            (let ([e (fxvector-ref v i)])
              (cond
                [(fx<=? e (car maxp))
                  (next-v (fx1+ i) maxp) ]
                [else 
                  (let next-m ([m maxp]) ;; insert element, dropping smallest
                    (cond
                      [(null? (cdr m))   ;; insert e at end
                        (let ([*e (cons e '())])
                          (set-cdr! m *e))
                        (next-v (fx1+ i) (cdr maxp)) ]
                      [(fx>? e (cadr m)) (next-m (cdr m)) ]
                      [else              ;; insert e after m
                        (let ([*e (cons e (cdr m))])
                          (set-cdr! m *e))
                        (next-v (fx1+ i) (cdr maxp)) ])) ])))) ]
      [rankedmax (if (fxzero? rankedmax)  1
                     rankedmax)])
    (do ( [i (fx1- n) (fx1- i)]
          [b (list) (if (fx>=? (fxvector-ref v i) rankedmax)
                      (cons i b)
                      b)])
        ((fxnegative? i) b))))
                                                                                            ;
(define (main args)                      ;; (String String) ->
  ;; $triadicmemory N P
  ;; store/delete/query triads
  (let* ( [N  (string->number (car args))]
          [P  (string->number (cadr args))]
          [tm (make-triadic-memory N P)])
  (let loop ()
    (let ([input-line (get-line (current-input-port))])
      (cond
        [(string=? input-line "quit") (exit 0) ]
        [else
          (let* ( [buf (string->list input-line)]
                  [del (char=? (car buf) #\-)]
                  [buf (if del (cdr buf) buf)]
                  [buf (if (char=? (car buf) #\{)  (cdr buf)
                           (error 'main "expecting {"))])
            (let*-values (
                [(x buf) (parse tm buf)]
                [(y buf) (parse tm buf)]
                [(z buf) (parse tm buf)])
              (cond
                [(and (pair? x) (pair? y) (pair? z))
                  (if del
                      (delete tm x y z)
                      (store  tm x y z)) ]
                [else (display
                        (map fx1+ (query tm x y z)))
                      (newline) ])
              (loop))) ])))))
                                                                                            ;
(define (store tm x y z)                 ;; TM SDR SDR SDR ->
  ;; update cube with triad x y z
  (let* ( [N      (tm-N tm)]
          [nybble (fxpositive? N)]
          [N      (fxabs N)]
          [cube   (tm-cube tm)])
    (do ([x x (cdr x)]) ((null? x))
      (let ([*x (fx* N (car x))])
        (do ([y y (cdr y)]) ((null? y))
          (let ([*xy (fx* N (fx+ *x (car y)))])
            (do ([z z (cdr z)]) ((null? z))
              (let ([*xyz (fx+ *xy (car z))])
                (bytevector-u8-set! cube *xyz
                  (let ([cube-xyz (bytevector-u8-ref cube *xyz)])
                    (when (and (fx=? (fxand #xF cube-xyz) #xF) nybble)
                      ;; overflow: convert to byte mode
                      (do ([i (fx1- (bytevector-length cube)) (fx1- i)]) ((fxnegative? i))
                        (bytevector-u8-set! cube i (fxand #xF (bytevector-u8-ref cube i))))
                      (tm-N! tm (fx- N)))
                    (fx1+ cube-xyz)))))))))
    (when (fxpositive? (tm-N tm))        ;; (tm-N may have been set by overflow)
      (do ([z z (cdr z)]) ((null? z))    ;; use upper nybble for z>y>x
        (let ([*z (fx* N (car z))])
          (do ([y y (cdr y)]) ((null? y))
            (let ([*zy (fx* N (fx+ *z (car y)))])
              (do ([x x (cdr x)]) ((null? x))
                (let ([*zyx (fx+ *zy (car x))])
                  (bytevector-u8-set! cube *zyx
                    (fx+ #x10 (bytevector-u8-ref cube *zyx))))))))))))
                                                                                            ;
(define (delete tm x y z)                ;; TM SDR SDR SDR ->
  ;; update cube, deleting triad x y z (won't switch back byte->nybble mode)
  (let* ( [N      (tm-N tm)]
          [nybble (fxpositive? N)]
          [N      (fxabs N)]
          [cube   (tm-cube tm)])
    (do ([x x (cdr x)]) ((null? x))
      (let ([*x (fx* N (car x))])
        (do ([y y (cdr y)]) ((null? y))
          (let ([*xy (fx* N (fx+ *x (car y)))])
            (do ([z z (cdr z)]) ((null? z))
              (let ([*xyz (fx+ *xy (car z))])
                (bytevector-u8-set! cube *xyz 
                  (let ([cube-xyz (bytevector-u8-ref cube *xyz)])
                    (if nybble
                      (fx+ (fxand #xF0 cube-xyz) (fxmax 0 (fx1- (fxand #xF cube-xyz))))
                      (fxmax 0 (fx1- cube-xyz)))))))))))
    (when nybble
      (do ([z z (cdr z)]) ((null? z))    ;; use upper nybble for z>y>x
        (let ([*z (fx* N (car z))])
          (do ([y y (cdr y)]) ((null? y))
            (let ([*zy (fx* N (fx+ *z (car y)))])
              (do ([x x (cdr x)]) ((null? x))
                (let ([*zyx (fx+ *zy (car x))])
                  (bytevector-u8-set! cube *zyx
                    (let ([cube-zyx (bytevector-u8-ref cube *zyx)])
                      (fx+ (fxmax 0 (fx- #x10 (fxand #xF0 cube-zyx)))
                           (fxand #xF cube-zyx)))))))))))))
                                                                                            ;
(define (query tm x y z)                 ;; TM SDR SDR SDR -> SDR
  ;; produce sdr for null x y z, given other two 
  (let* ( [N    (tm-N tm)]
          [mask (if (fxpositive? N)  #xF  #xFF)]
          [N    (fxabs N)]
          [cube (tm-cube tm)]
          [q    (make-fxvector N 0)])
    (cond
      [(and (pair? x) (pair? y) (null? z))  ;; recall z
        (do ([x x (cdr x)]) ((null? x))
          (let ([*x (fx* N (car x))])
            (do ([y y (cdr y)]) ((null? y))
              (let ([*xy (fx* N (fx+ *x (car y)))])
                (do ([*xyz *xy (fx+ 4 *xyz)] [k 0 (fx+ 4 k)]) ((fx=? k N))
                  ;; unroll incrementing 4 elements of q from 4 cube bytes
                  (let ([nn   (bytevector-u32-native-ref cube *xyz)]
                        [k1   (fx1+ k)]
                        [k2   (fx+ 2 k)]
                        [k3   (fx+ 3 k)])
                    (fxvector-set! q k  (fx+ (fxvector-ref q k)
                        (fxand mask nn)))
                    (fxvector-set! q k1 (fx+ (fxvector-ref q k1)
                        (fxand mask (fxarithmetic-shift-right nn 8))))
                    (fxvector-set! q k2 (fx+ (fxvector-ref q k2)
                        (fxand mask (fxarithmetic-shift-right nn 16))))
                    (fxvector-set! q k3 (fx+ (fxvector-ref q k3)
                        (fxand mask (fxarithmetic-shift-right nn 24)))))))))) ]
      [(and (pair? x) (null? y) (pair? z))  ;; recall y
        (let ([z (list->fxvector z)]
              [P (tm-P tm)])
          (do ([x x (cdr x)]) ((null? x))
            (let ([*x (fx* N N (car x))])
              (do ([j 0 (fx1+ j)] [*xy *x (fx+ *xy N)]) ((fx=? j N))
                (fxvector-set! q j 
                  (do ( [k 0 (fx1+ k)]
                        [qj (fxvector-ref q j) (fx+ qj
                            (fxand mask (bytevector-u8-ref cube (fx+ *xy (fxvector-ref z k)))))])
                      ((fx=? k P) qj))))))) ]
      [(and (null? x) (pair? y) (pair? z)   ;; recall x
            (fxpositive? (tm-N tm)))        ;; (using upper nybble)
        (do ([z z (cdr z)]) ((null? z))
          (let ([*z (fx* N (car z))])
            (do ([y y (cdr y)]) ((null? y))
              (let ([*zy (fx* N (fx+ *z (car y)))])
                (do ([i 0 (fx1+ i)] [*zyx *zy (fx1+ *zyx)]) ((fx=? i N))
                  (fxvector-set! q i (fx+ (fxvector-ref q i)
                      (fxarithmetic-shift-right (bytevector-u8-ref cube *zyx) 4)))))))) ]
      [(and (null? x) (pair? y) (pair? z))  ;; recall x (using whole byte)
        (let ([z (list->fxvector z)]
              [P (tm-P tm)])
          (do ([i 0 (fx1+ i)] [*x 0 (fx+ *x (fx* N N))]) ((fx=? i N))
            (do ([y y (cdr y)]) ((null? y))
              (let ([*xy (fx+ *x (fx* N (car y)))])
                (fxvector-set! q i 
                  (do ( [k 0 (fx1+ k)]
                        [qi (fxvector-ref q i) (fx+ qi
                            (bytevector-u8-ref cube (fx+ *xy (fxvector-ref z k))))])
                      ((fx=? k P) qi))))))) ]
      [else (error 'query "one of triad should be null") ])
    (binarize q N (tm-P tm))))

(define-syntax expect                    ;; ((X ... -> Y) X ...) Y -> Error |
  ;; check that function application(s) to arguments match expected values
  (lambda (x)                            
    (syntax-case x (=>)                  ;; (expect [(fn args) => expected ] ... )
      [ (_ (expr => expected) ...)       ;; expr matches (fn args)
        #'(begin 
            (let ([result expr])         ;; eval expr once, no output if check passes
              #;(when (equal? result expected) (display "."))
              (unless (equal? result expected)
                (for-each display 
                  `("**" expr #\newline 
                    "  expected: " ,expected #\newline 
                    "  returned: " ,result  #\newline))
                (exit))) ...)])))

;; smoke test
(let ([tm (make-triadic-memory 9 3)])
  (store tm '(0 1 2) '(3 4 5) '(6 7 8))
  (expect [(query tm '()      '(3 4 5) '(6 7 8)) => '(0 1 2)]
          [(query tm '(0 1 2) '()      '(6 7 8)) => '(3 4 5)]
          [(query tm '(0 1 2) '(3 4 5) '()     ) => '(6 7 8)]) )

)

#| #;(uncomment to enable running at command line)

(import (triadicmemory))
(main (command-line-arguments))

|#
