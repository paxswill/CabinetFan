#ifndef FAN_MOVING_AVERAGE_H
#define FAN_MOVING_AVERAGE_H

template<typename T = float, uint8_t N = 10>
class MovingAverage {
  public:
    /* Create a `MovingAverage` that averages the last N values.
    */
    MovingAverage():
      count(0),
      sum(0),
      values {0}
    {
      current = values;
    };

    /* Add a new value to the moving average.
     */
    void push(T new_value) {
      if (count == N) {
        // Need to subtract the value being replaced first
        sum -= *current;
      } else {
        // Keep track of the number of elements as we're filling the list.
        count++;
      }
      *current = new_value;
      sum += new_value;
      // Increment `current`
      current++;
      if (current >= (values + N)) {
        current = values;
      }
    }

    /* Calculate the current moving average.
     */
    T current_value() {
      return sum / T(count);
    }

    /* Add a new value and calculate the current movign average.
     */
    T update(T new_value) {
      push(new_value);
      return current_value();
    }

  private:
    // The current number of values in the average.
    uint8_t count;

    /* The sum of all values.
     * This is an optimization so that adding only happens on adding an object
     * to the list, instead of every value being summed every time the average
     * is being retrieved.
     */
    T sum;

    /* The values that are being averaged.
     * This is a circular buffer, accessed through the `current` pointer.
     */
    T values[N];

    // The address of the next value to be added to the buffer.
    T * current;
};

#endif