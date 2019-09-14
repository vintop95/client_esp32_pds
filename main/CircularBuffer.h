/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

/**
 * Struct that limits its memory used to avoid 
 * STACK OVERFLOW
 */
template <class T>
class CircularBuffer
{
private:
    std::map<int,T> buffer;
    int max_size;
    int index_first; //POP
    int index_next; //PUSH
    int m_size;
    int elements_ever_inserted;

public:
    CircularBuffer(): 
    index_first(0), index_next(0), m_size(0), elements_ever_inserted(0){

        int bytes_to_leave = 8192;
        // CALCULATE MAX SIZE
        // https://esp32.com/viewtopic.php?t=3802
        // https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/include/heap/esp_heap_caps.h
        // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/mem_alloc.html
        int free_size_bytes = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_DEFAULT: %d", free_size_bytes);
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_8BIT: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_32BIT: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT));
        int max_elem_size = sizeof(T);
        ESP_LOGI("CircularBuffer", "SIZE OF TYPE: %d", max_elem_size);

        if(free_size_bytes < bytes_to_leave){
            throw std::bad_alloc();
        }
        
        int size_allocable = (free_size_bytes - bytes_to_leave);  

        max_size = size_allocable / max_elem_size;
        ESP_LOGI("CircularBuffer", "LEAVE %d BYTES FREE", bytes_to_leave);
        ESP_LOGI("CircularBuffer", "CIRCULAR BUFFER OF MAX %d ELEMENTS CONSTRUCTED", max_size);
    }

    int size() const{
        //ESP_LOGI("CircularBuffer", "CURRENT SIZE=%d, ELEM_EVER_HAD=%d", m_size, elements_ever_inserted);
        return m_size < max_size ? m_size : max_size;
    }

    void increaseSize(){
        if(m_size < max_size){
            m_size++;
        }
    }

    void decreaseSize(){
        if(m_size > 0){
            m_size--;
        }
    }

    bool isPoppable() const{
        return !(index_next == index_first);
    }

    int increaseWithModule(int val) const{
        int new_val = (val + 1) % max_size;
        // ESP_LOGI("CircularBuffer", "INCREASE INDEX FROM %d TO %d", val, new_val);
        return new_val;
    }

    void reset(){
        ESP_LOGI("CircularBuffer", "CLEAR BUFFER");
        buffer.clear();
        index_first = 0;
        index_next = index_first;
        m_size = 0;
    }

    void push(const T& t){
        ESP_LOGI("CircularBuffer", "PUSH RECORD");
        increaseSize();
        buffer[index_next] = t;
        // ESP_LOGI("CircularBuffer", "FOR INDEX_NEXT:");
        index_next = increaseWithModule(index_next);
        // index_next non deve mai superare index_first
        // da dietro
        if (!isPoppable()){
            // ESP_LOGI("CircularBuffer", "FOR INDEX_FIRST:");
            index_first = increaseWithModule(index_first);
        }
        elements_ever_inserted++;
        ESP_LOGI("CircularBuffer", "CURRENT SIZE=%d, ELEM_EVER_HAD=%d", m_size, elements_ever_inserted);
    }    

    T pop(){
        if(isPoppable()){
            // ESP_LOGI("CircularBuffer", "POP RECORD");
            decreaseSize();
            T& t = buffer[index_first];
            // ESP_LOGI("CircularBuffer", "FOR INDEX_FIRST:");
            index_first = increaseWithModule(index_first);
            return t;
        }else{
            throw std::bad_typeid();
        }
    }

};

#endif