#!/bin/bash
(echo $$ reading; read a < foo0; echo $$ finishing) &
(echo $$ reading; read a < foo1; echo $$ finishing) &
(echo $$ reading; read a < foo2; echo $$ finishing) &
sleep 1000
wait
