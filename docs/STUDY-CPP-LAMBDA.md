## Modern C++ `[&]`의 정확한 의미

### 1. `[&]`는 캡처 방식, `bus_`는 캡처 대상

```cpp
bus_->waitForState(target, timeout,
                   [&]  // ← 이것은 "모든 외부 변수를 참조로 캡처하라"는 지시
                   {
                       bus_->sendNop([](auto const&) {});  // ← 여기서 bus_를 사용
                       bus_->finalizeDatagrams();
                       bus_->processAwaitingFrames();
                   });
```

**`[&]`는 "어떻게 캡처할 것인가"를 지정하는 것이고, `bus_`는 "무엇을 캡처할 것인가"입니다.**

### 2. C에서의 동등한 개념으로 설명

**C++ 람다:**
```cpp
[&] {  // 모든 외부 변수를 참조로 캡처
    bus_->sendNop([](auto const&) {});
}
```

**C에서의 동등한 구현:**
```c
// [&]에 해당하는 부분: 참조로 캡처
typedef struct {
    kickcat_bus_t* bus;  // bus_를 참조로 캡처 (포인터로)
    // 다른 외부 변수들도 참조로 캡처
} callback_data_t;

void callback(void* user_data) {
    callback_data_t* data = (callback_data_t*)user_data;
    // data->bus는 원본 bus_를 가리키는 포인터 (참조와 동일한 효과)
    bus_send_nop(data->bus, NULL);
}
```

### 3. 다른 캡처 방식과 비교

```cpp
// 1. [&] - 모든 외부 변수를 참조로 캡처
[&] {
    bus_->sendNop([](auto const&) {});  // bus_를 참조로 접근
}

// 2. [=] - 모든 외부 변수를 값으로 복사
[=] {
    // bus_를 값으로 복사 (하지만 bus_는 포인터이므로 포인터 값만 복사)
    bus_->sendNop([](auto const&) {});
}

// 3. [&bus_] - bus_만 참조로 캡처
[&bus_] {
    bus_->sendNop([](auto const&) {});  // bus_만 참조로 접근
}

// 4. [bus_] - bus_만 값으로 복사
[bus_] {
    bus_->sendNop([](auto const&) {});  // bus_를 값으로 복사
}
```

### 4. C에서의 동등한 구현들

**`[&]` (모든 변수 참조 캡처):**
```c
typedef struct {
    kickcat_bus_t* bus;        // 참조 (포인터)
    int local_var;             // 참조 (포인터)
    // 모든 외부 변수들을 포인터로
} all_ref_context_t;

void callback_all_ref(void* user_data) {
    all_ref_context_t* ctx = (all_ref_context_t*)user_data;
    bus_send_nop(ctx->bus, NULL);  // 원본 bus_를 참조
}
```

**`[=]` (모든 변수 값 복사):**
```c
typedef struct {
    kickcat_bus_t* bus;        // 값 복사 (포인터 값만 복사)
    int local_var;             // 값 복사
    // 모든 외부 변수들을 값으로 복사
} all_copy_context_t;

void callback_all_copy(void* user_data) {
    all_copy_context_t* ctx = (all_copy_context_t*)user_data;
    bus_send_nop(ctx->bus, NULL);  // 복사된 포인터 값 사용
}
```

**`[&bus_]` (bus_만 참조 캡처):**
```c
typedef struct {
    kickcat_bus_t* bus;        // bus_만 참조로
    // 다른 변수들은 캡처하지 않음
} bus_only_ref_context_t;

void callback_bus_only_ref(void* user_data) {
    bus_only_ref_context_t* ctx = (bus_only_ref_context_t*)user_data;
    bus_send_nop(ctx->bus, NULL);
}
```

### 5. 실제 메모리 관점에서

```cpp
class MasterController {
    kickcat::Bus* bus_;  // 멤버 변수
    
    void someFunction() {
        int local_var = 42;
        
        // [&]는 bus_와 local_var 모두를 참조로 캡처
        bus_->waitForState(target, timeout, [&] {
            // 여기서 bus_는 원본 객체의 멤버를 참조
            // local_var도 원본 변수를 참조
            bus_->sendNop([](auto const&) {});
        });
    }
};
```

**C에서의 동등한 구현:**
```c
typedef struct {
    kickcat_bus_t* bus;
    int local_var;
} master_controller_t;

void some_function(master_controller_t* self) {
    int local_var = 42;
    
    // [&]에 해당: 모든 외부 변수를 참조로 캡처
    callback_context_t ctx = {
        .bus = self->bus,        // 원본 bus_를 참조 (포인터)
        .local_var = &local_var  // 원본 local_var를 참조 (포인터)
    };
    
    bus_wait_for_state(self->bus, target, timeout, callback, &ctx);
}

void callback(void* user_data) {
    callback_context_t* ctx = (callback_context_t*)user_data;
    
    // ctx->bus는 원본 self->bus를 가리킴 (참조와 동일한 효과)
    bus_send_nop(ctx->bus, NULL);
}
```

## 요약

- **`&`는 `bus_`의 참조가 아님**
- **`[&]`는 "모든 외부 변수를 참조로 캡처하라"는 지시**
- **`bus_`는 캡처되는 대상 중 하나**
- **C에서는 구조체의 포인터 멤버로 참조와 동일한 효과를 구현**

`[&]`는 캡처 방식(how to capture)이고, `bus_`는 캡처 대상(what to capture)입니다.