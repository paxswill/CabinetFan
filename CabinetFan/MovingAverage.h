#ifndef FAN_MOVING_AVERAGE_H
#define FAN_MOVING_AVERAGE_H

#include "Arduino.h"

template<typename T = float, uint8_t N = 10>
class MovingAverage {
  public:
    /* Create a `MovingAverage` that averages the last N values.
    */
    MovingAverage():
      head(NULL),
      tail(NULL),
      current_length(0),
      sum(0)
    {};

    /* Add a new value to the moving average.
     */
    void push(T new_value) {
      if (current_length >= N) {
        // Subtract and delete the old node
        Node * old_tail = tail;
        sum -= old_tail->value();
        tail = old_tail->pop();
        current_length -= 1;
      }
      // Push a new head node, and update the sum
      if (head == NULL) {
        head = new Node(new_value);
        tail = head;
      } else {
        head = head->push(new_value);
      }
      sum += head->value();
      current_length +=1;
    }

    /* Calculate the current moving average.
     */
    T current_value() {
      return sum / T(current_length);
    }

    /* Add a new value and calculate the current movign average.
     */
    T update(T new_value) {
      push(new_value);
      return current_value();
    }

  private:
    class Node {
      public:
        Node(T value): _value(value), _next(NULL), _prev(NULL) {};

        ~Node() {
          if (_next != NULL) {
            _next->_prev = _prev;
          }
          if (_prev != NULL) {
            _prev->_next = _next;
          }
        }

        // Prepend a new node to this linked list, returning the new head.
        Node * push(Node * new_node) {
          Node * old_head = this;
          while (old_head->_prev != NULL) {
            old_head = old_head->_prev;
          }
          new_node->_next = old_head;
          old_head->_prev = new_node;
          return new_node;
        }

        // Prepend a new value to this linked list, returning the new head.
        Node * push(T new_value) {
          Node * new_node = new Node(new_value);
          return this->push(new_node);
        }

        // Delete the last node of this linked list, returning the new tail.
        Node * pop() {
          Node * old_tail = this;
          while (old_tail->_next != NULL) {
            old_tail = old_tail->_next;
          }
          Node * new_tail = old_tail->_prev;
          delete old_tail;
          return new_tail;
        }

        // Retrieve value of this node.
        T value() {
          return this->_value;
        }

        Node * next() {
          return this->_next;
        }

        Node * prev() {
          return this->_prev;
        }
      private:
        T _value;
        Node * _next;
        Node * _prev;
    };

    Node *head;
    Node *tail;
    uint8_t current_length;
    T sum;
};

#endif