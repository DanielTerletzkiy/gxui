#ifndef EPDPAGE_H
#define EPDPAGE_H

/**
 * @file EPDPage.h
 * Base page container that hosts interactables and manages focus.
 *
 * A Page is itself an Interactable and can contain multiple child
 * interactables. Provides helpers to add/find/select/activate items
 * by ID or index and a hook for when the page is opened.
 */

#include <string> // for std::string and std::hash
#include <EPDInteractable.h>

//#include <EPDMenu.h>
namespace std
{
    template <>
    struct hash<String>
    {
        size_t operator()(const String& s) const noexcept
        {
            // Convert to a std::string and use its hash
            return std::hash<std::string>()(std::string(s.c_str()));
        }
    };
}

namespace EPD
{
    class Page : public Interactable
    {
    public:
        explicit Page() = default;

        /*
         * @brief Executes when the page will get "opened"
         */
        virtual void onPageOpened()
        {
        }

        [[nodiscard]] virtual String getTitle() const = 0;

        /**
         * @brief Determines if the page should render content when it's not focused.
         *
         * This method controls the visibility of page content when the page
         * doesn't have focus. By default, it returns true, meaning the page
         * content will always render regardless of interactable focus.
         * Override this method to control the rendering of unfocused content.
         *
         * True can degrade performance as background tasks for the whole page have to be redone every time.
         * False can cause issues with rendering interactables that are not visible. For example: overlapping borders
         *
         * @return true if unfocused content should be rendered, false otherwise
         */
        [[nodiscard]] virtual bool shouldRenderUnfocusedContent() const
        {
            return true;
        }

        Interactable* addInteractable(std::unique_ptr<Interactable> interactable, const bool focusable = true)
        {
            const String id = interactable->getId();
            if (interactableMap.find(id) != interactableMap.end())
            {
                Serial.printf("Duplicate interactable ID: %s\n", id.c_str());
                return nullptr;
            }

            if (focusable)
            {
                interactable->enableInteraction();
            }
            else
            {
                interactable->disableInteraction();
            }

            interactableMap[id] = interactables.size();
            interactables.push_back(std::move(interactable));

            return interactable.get();
        }

        Interactable* getInteractable(const String& id)
        {
            const auto it = interactableMap.find(id);
            return (it != interactableMap.end()) ? interactables[it->second].get() : nullptr;
        }

        Interactable* getInteractable(const size_t index)
        {
            return (index < interactables.size()) ? interactables[index].get() : nullptr;
        }

        bool selectInteractableById(const String& id)
        {
            const auto it = interactableMap.find(id);
            if (it != interactableMap.end())
            {
                tempInteractableIndex = currentInteractableIndex;
                setSelectedIndex(it->second);
                return true;
            }
            return false;
        }

        bool activateInteractableById(const String& id)
        {
            if (selectInteractableById(id))
            {
                getCurrentInteractable()->activate();
                return true;
            }
            return false;
        }


        [[nodiscard]] const std::vector<std::unique_ptr<Interactable>>& getInteractables() { return interactables; }
        [[nodiscard]] int getInteractablesSize() const { return interactables.size(); }


        [[nodiscard]] Interactable* getCurrentInteractable() const
        {
            if (currentInteractableIndex < 0)
            {
                return nullptr;
            }
            return interactables[currentInteractableIndex].get();
        }

        void onActionLeft() override
        {
        };

        void onActionRight() override
        {
        };

        void onActionUp() override
        {
            if (interactables.empty()) return;

            int newIndex = currentInteractableIndex;
            do
            {
                newIndex--;
                if (newIndex < 0) return;
            }
            while (!interactables[newIndex]->getIsInteractable());

            setSelectedIndex(newIndex);
        };

        void onActionDown() override
        {
            if (interactables.empty()) return;

            int newIndex = currentInteractableIndex;
            do
            {
                newIndex++;
                if (newIndex >= getInteractablesSize()) return;
            }
            while (!interactables[newIndex]->getIsInteractable());

            setSelectedIndex(newIndex);
        };

        void onAction() override
        {
            const auto currentInteractable = getCurrentInteractable();
            if (currentInteractable == nullptr)
            {
                return;
            }

            currentInteractable->onAction();
        };

        void resetFocus()
        {
            if (const auto currentInteractable = getCurrentInteractable())
            {
                currentInteractable->deselect();
                currentInteractable->deactivate();
            }

            if (tempInteractableIndex >= 0)
            {
                setSelectedIndex(tempInteractableIndex);
            }
            else
            {
                currentInteractableIndex = -1;
                for (int i = 0; i < interactables.size(); i++)
                {
                    if (interactables[i]->getIsInteractable())
                    {
                        setSelectedIndex(i);
                        break;
                    }
                }
            }
        }

        void setSelectedIndex(const int index)
        {
            if (const auto currentInteractable = getCurrentInteractable(); currentInteractable != nullptr)
            {
                currentInteractable->deselect();
                currentInteractable->deactivate();
            }

            currentInteractableIndex = index;

            const auto newInteractable = getCurrentInteractable();
            if (newInteractable == nullptr)
            {
                return;
            }
            newInteractable->select();
        }

        [[nodiscard]] int getSelectedIndex() const
        {
            return currentInteractableIndex;
        }

    protected:
        void onModalClose()
        {
            resetFocus();
        }

        std::unordered_map<String, size_t> interactableMap{};
        std::vector<std::unique_ptr<Interactable>> interactables;
        int currentInteractableIndex = -1;
        int tempInteractableIndex = -1;
    };
}
#endif
