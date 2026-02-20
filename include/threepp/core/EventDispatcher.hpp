// https://github.com/mrdoob/three.js/blob/r129/src/core/EventDispatcher.js

#ifndef THREEPP_EVENTDISPATCHER_HPP
#define THREEPP_EVENTDISPATCHER_HPP

#include <any>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>


namespace threepp {

    struct Event {

        const std::string type;
        std::any target;
    };

    struct EventListener {

        virtual void onEvent(Event& event) = 0;

        virtual ~EventListener() = default;
    };

    struct LambdaEventListener: EventListener {

        explicit LambdaEventListener(std::function<void(Event&)> f): f_(std::move(f)) {}

        void onEvent(Event& event) override {
            f_(event);
        }

    private:
        std::function<void(Event&)> f_;
    };

    class EventDispatcher {

    public:
        void addEventListener(const std::string& type, EventListener& listener);

        bool hasEventListener(const std::string& type, const EventListener& listener);

        void removeEventListener(const std::string& type, const EventListener& listener);

        void dispatchEvent(const std::string& type, std::any target = {});

        virtual ~EventDispatcher() = default;

    private:
        std::unordered_map<std::string, std::vector<EventListener*>> listeners_;
    };

}// namespace threepp

#endif//THREEPP_EVENTDISPATCHER_HPP
