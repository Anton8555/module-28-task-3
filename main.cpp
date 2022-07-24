#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>
#include <cassert>
using namespace std;





// meal options
enum Dish {NONE, PIZZA, SOUP, STEAK, SALAD, SUSHI};

// order data structure
struct Order{
    int id;     // Order number
    Dish dish;  // dish code
};

// queue of orders
queue<Order> queueOrder;
mutex queueOrder_access;

// queue of completed orders for issuance
queue<Order> queueOnExtradition;
mutex queueOnExtradition_access;

// condition variable: message output queue to the console
bool fOut;
mutex fOut_access;

// condition variable: terminate threads (true-terminate)
bool fThreadReturn;
mutex fReturn_access;







/*!
 * @brief The function of generating an integer random number in the interval [min; max].
 * @param min - the minimum value of the interval;
 * @param max - the maximum value of the interval;
 * @return Returns the generated number.
 */
int randIntRange(int min, int max) {
    assert( min <= max );
    return rand() % (max - min + 1) + min;
}

/*!
 * @brief The function calculates the text designation of the dish code designation.
 * @param dish - code designation of the dish;
 * @return Returns the name of the dish as text.
 */
string toDishStr(Dish dish) {
    switch( dish ) {
        case NONE:  return "None";
        case PIZZA: return "Pizza";
        case SOUP:  return "Soup";
        case STEAK: return "Steak";
        case SALAD: return "Salad";
        case SUSHI: return "Sushi";
        default: return "None";
    }
}

/*!
 * @brief Emulation thread subroutine receiving order.
 */
void orderReceipt() {
    // data
    int countID = 0;  // order counter

    while(true) {
        // data
        Dish dish;

        // receipt of orders every 5-10 seconds.
        this_thread::sleep_for(chrono::seconds(randIntRange(5, 10)));

        // adding a new order to the queue
        dish = (Dish) randIntRange(1, 5);
        queueOrder_access.lock();
        queueOrder.push({++countID, (Dish)dish});
        queueOrder_access.unlock();

        // displaying a message about the receipt of an order
        fOut_access.lock();
        cout
            << "\nNew order arrival:\n"
            << "\tOrder id: " << countID << endl
            << "\tDish: " << toDishStr(dish) << endl;
        fOut_access.unlock();

        // terminate the execution of the current thread on a condition variable
        fReturn_access.lock();
        if( fThreadReturn ) {
            fReturn_access.unlock();
            return;
        }
        fReturn_access.unlock();
    }
}

/*!
 * @brief Kitchen emulation thread subroutine.
 */
void kitchen() {
    while(true) {
        // data
        int time = randIntRange(5, 15);
        Order order;

        // taking the order from the queue by the kitchen
        while(true) {
            // trying to take an order for execution
            queueOrder_access.lock();
            if ( !queueOrder.empty() ) {
                order = queueOrder.front();
                queueOrder.pop();
                queueOrder_access.unlock();
                break;
            }
            queueOrder_access.unlock();

            // a little pause
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        // displaying a message about taking an order for execution
        fOut_access.lock();
        cout
                << "\nTaking an order for execution:\n"
                << "\tOrder id: " << order.id << endl
                << "\tDish: " << toDishStr(order.dish) << endl
                << "Cooking time: " << time << endl;
        fOut_access.unlock();

        // "cooking"
        this_thread::sleep_for(chrono::seconds(time));

        // return of the finished dish for delivery
        queueOnExtradition_access.lock();
        queueOnExtradition.push(order);
        queueOnExtradition_access.unlock();

        // displaying a message about the return of the finished dish for delivery
        fOut_access.lock();
        cout
                << "\nReturn of the finished dish for delivery:\n"
                << "\tOrder id: " << order.id << endl
                << "\tDish: " << toDishStr(order.dish) << endl;
        fOut_access.unlock();

        // terminate the execution of the current thread on a condition variable
        fReturn_access.lock();
        if( fThreadReturn ) {
            fReturn_access.unlock();
            return;
        }
        fReturn_access.unlock();
    }
}

/*!
 * @brief A subroutine for emulating the work of a courier.
 */
void courier() {
    // data
    int count = 0;  // counter of sent orders

    //
    while(true) {
        // courier arrives every 30 seconds
        this_thread::sleep_for(chrono::seconds(30));

        // the courier picks up ready-made dishes at the delivery and delivers them to customers
        queueOrder_access.lock();
        fOut_access.lock();
        cout << "\nThe courier picks up the following ready meals:\n";
        while( !queueOnExtradition.empty() ) {
            // pick up the finished dish
            Order order = queueOnExtradition.front();
            queueOnExtradition.pop();

            // displaying a message about a taken ready dish
            cout
                    << "\tOrder id: " << order.id << endl
                    << "\tDish: " << toDishStr(order.dish) << endl;

            // count of sent orders
            count++;
        }
        fOut_access.unlock();
        queueOrder_access.unlock();

        // terminate the work of the threads under the given condition
        if ( count >= 10 ) {
            fReturn_access.lock();
            fThreadReturn = true;
            fReturn_access.unlock();
            return;
        }
    }
}








int main() {
    // RNG initialization
    srand(time(nullptr));

    // starting threads
    fThreadReturn = false;
    thread queueOrder1(orderReceipt);
    thread queueOrder2(kitchen);
    thread courierOrder(courier);

    // waiting for threads to complete
    // wait for the courier thread to finish first, because it manages thread termination.
    if( courierOrder.joinable() )
        courierOrder.join();
    if( queueOrder1.joinable() )
        queueOrder1.join();
    if( queueOrder2.joinable() )
        queueOrder2.join();

    // the end
    cout << "\nEnd of Program.\n";
    return 0;
}
