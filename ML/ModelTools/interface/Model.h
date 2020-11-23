/*
 * Generic model for ML purposes.
 */

#ifndef ML_MODELTOOLS_INTERFACE_MODEL_H_
#define ML_MODELTOOLS_INTERFACE_MODEL_H_

#include "common.h"
#include "InputOutput.h"

namespace edm {

  namespace ml {

    class Model {
    public:
      Model(const std::string& name, const std::string& fileName = "");

      Model(const Model& m) : name_(m.name_), inputs_(m.inputs_), outputs_(m.outputs_) {}

      Model() = delete;

      virtual ~Model() {}

      const std::string& getName() const { return name_; }

      bool hasInput(const std::string& name) const { return inputs_.find(name) != inputs_.end(); }

      bool hasOutput(const std::string& name) const { return outputs_.find(name) != outputs_.end(); }

      size_t getNInputs() const { return inputs_.size(); }

      size_t getNOutputs() const { return outputs_.size(); }

      template <typename T, typename... Args>
      Input<T>& addInput(const std::string& name, const BatchableShape& shape, Args&&... args);

      template <typename T, typename... Args>
      Output<T>& addOutput(const std::string& name, const BatchableShape& shape, Args&&... args);

      template <typename T>
      Input<T>& getInput(const std::string& name);

      template <typename T>
      Output<T>& getOutput(const std::string& name);

      InputMap::const_iterator beginInputs() const { return inputs_.begin(); }

      InputMap::iterator beginInputs() { return inputs_.begin(); }

      InputMap::const_iterator endInputs() const { return inputs_.end(); }

      InputMap::iterator endInputs() { return inputs_.end(); }

      OutputMap::const_iterator beginOutputs() const { return outputs_.begin(); }

      OutputMap::iterator beginOutputs() { return outputs_.begin(); }

      OutputMap::const_iterator endOutputs() const { return outputs_.end(); }

      OutputMap::iterator endOutputs() { return outputs_.end(); }

      void loadJson(const std::string& fileName);

    protected:
      const std::string name_;
      InputMap inputs_;
      OutputMap outputs_;
    };

    template <typename T, typename... Args>
    Input<T>& Model::addInput(const std::string& name, const BatchableShape& shape, Args&&... args) {
      if (hasInput(name)) {
        throw cms::Exception("DuplicateInputName")
            << "input '" << name << "' already registered in model '" << getName() << "'";
      }
      auto ret = inputs_.emplace(name, Input<T>(name, shape, std::forward<Args>(args)...));
      return std::get<Input<T>>((*(ret.first)).second);
    }

    template <typename T, typename... Args>
    Output<T>& Model::addOutput(const std::string& name, const BatchableShape& shape, Args&&... args) {
      if (hasOutput(name)) {
        throw cms::Exception("DuplicateOutputName")
            << "output '" << name << "' already registered in model '" << getName() << "'";
      }
      auto ret = outputs_.emplace(name, Output<T>(name, shape, std::forward<Args>(args)...));
      return std::get<Output<T>>((*(ret.first)).second);
    }

    template <typename T>
    Input<T>& Model::getInput(const std::string& name) {
      auto it = inputs_.find(name);
      if (it == inputs_.end()) {
        throw cms::Exception("UnknownInput") << "no input '" << name << "' registered in model '" << getName() << "'";
      }
      if (!checkVarContainerType<T, VarInput>(it->second)) {
        throw cms::Exception("TypeMismatch") << "variable input with index " << it->second.index()
                                             << " does not match template type '" << typeid(T).name() << "'";
      }
      return std::get<Input<T>>(it->second);
    }

    template <typename T>
    Output<T>& Model::getOutput(const std::string& name) {
      auto it = outputs_.find(name);
      if (it == outputs_.end()) {
        throw cms::Exception("UnknownOutput") << "no output '" << name << "' registered in model '" << getName() << "'";
      }
      if (!checkVarContainerType<T, VarOutput>(it->second)) {
        throw cms::Exception("TypeMismatch") << "variable output with index " << it->second.index()
                                             << " does not match template type '" << typeid(T).name() << "'";
      }
      return std::get<Output<T>>(it->second);
    }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_MODEL_H_
