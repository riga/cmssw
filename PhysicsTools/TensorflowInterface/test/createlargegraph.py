# -*- coding: utf-8 -*-

"""
Test script to create a more complex graph for testing purposes at bin/data.
"""


import os
import tensorflow as tf


thisdir = os.path.dirname(os.path.abspath(__file__))
datadir = os.path.join(os.path.dirname(thisdir), "bin", "data")

x_ = tf.placeholder(tf.float32, [None, 100], name="input")

prev_output = x_
prev_units = 100
nn = 5 * [200]

for units in nn:
    W = tf.Variable(tf.random_normal([prev_units, units]))
    b = tf.Variable(tf.random_normal([units]))
    h = tf.nn.elu(tf.matmul(prev_output, W) + b)
    prev_units = units
    prev_output = h

W_last = tf.Variable(tf.random_normal([prev_units, 10]))
b_last = tf.Variable(tf.random_normal([10]))
y  = tf.nn.softmax(tf.matmul(prev_output, W_last) + b_last, name="output")

sess = tf.Session()
sess.run(tf.global_variables_initializer())

print(sess.run(y, feed_dict={x_: [range(100)]})[0])

saver = tf.train.Saver()
saver.save(sess, os.path.join(datadir, "largegraph"))
