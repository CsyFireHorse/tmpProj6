#ifndef _ZMC_IO_MANAGER_HPP_
#define _ZMC_IO_MANAGER_HPP_

#include <cstdint>
#include <vector>

#include "io/IoMetadata.hpp"
#include "utils/zmc_pio_offset_layout.h"

namespace zmc {

struct IoObjectKey {
    uint16_t slave_idx;
    uint16_t index;
    uint8_t subindex;
};

class IoManager {
   public:
    /* Construct an empty IO manager. */
    IoManager();

    /* Release the private implementation. */
    ~IoManager();

    /* Disallow copying. */
    IoManager(const IoManager&) = delete;
    IoManager& operator=(const IoManager&) = delete;

    /* Copy PDO information and allocate zeroed IO storage. */
    int apply_pio_layout(const ZmcPioOffsetLayout* layout);

    /* Bind process images and refresh all registered PDO digital inputs. */
    void bind_process_image(const void* input, void* output);

    /* Flush compiled PDO Shadow operations to the bound output image. */
    void restore_retained_outputs();

    /* Read input bytes directly at one process-image offset. */
    int read_input(uint16_t byte_offset, uint8_t byte_len, uint32_t* value) const;

    /* Read output bytes directly at one process-image offset. */
    int read_output(uint16_t byte_offset, uint8_t byte_len, uint32_t* value) const;

    /* Write output bytes directly without changing the PDO Shadow. */
    int write_output(uint16_t byte_offset, uint8_t byte_len, uint32_t value);

    /* Read one input object by its concrete PDO identity. */
    int read_input(const IoObjectKey& key, uint32_t* value) const;

    /* Read one output object by its concrete PDO identity. */
    int read_output(const IoObjectKey& key, uint32_t* value) const;

    /* Write one output PDO object directly without changing the PDO Shadow. */
    int write_output(const IoObjectKey& key, uint32_t value);

    /* Register a digital-input alias for a one-bit PDO object. */
    int register_di_alias(uint32_t io_number, const IoObjectKey& key);

    /* Register one digital-input bit selected by a one-hot mask. */
    int register_di_alias(uint32_t io_number, const IoObjectKey& key, uint32_t mask);

    /* Register a digital-output alias for a one-bit PDO object. */
    int register_do_alias(uint32_t io_number, const IoObjectKey& key);

    /* Register one PDO digital output bit selected by a one-hot mask. */
    int register_do_alias(uint32_t io_number, const IoObjectKey& key, uint32_t mask);

    /* Register an analog-input alias. */
    int register_ai_alias(uint32_t io_number, const IoObjectKey& key);

    /* Register an analog-output alias. */
    int register_ao_alias(uint32_t io_number, const IoObjectKey& key);

    /* Register a zero-initialized virtual digital input. */
    int register_virtual_di(uint32_t io_number);

    /* Register a zero-initialized virtual digital output. */
    int register_virtual_do(uint32_t io_number);

    /* Register a zero-initialized virtual analog input. */
    int register_virtual_ai(uint32_t io_number);

    /* Register a zero-initialized virtual analog output. */
    int register_virtual_ao(uint32_t io_number);

    /* Copy all registered PDO and virtual IO metadata. */
    int get_alias_metadata(std::vector<IoMetadataEntry>& out) const;

    /* Clear aliases, IO Banks, Shadows, and compiled plans. */
    void clear_aliases();

    /* Read a current PDO input or virtual digital-input value. */
    int read_di(uint32_t io_number, bool* value) const;

    /* Read up to 32 cached digital inputs from the packed DI Bank. */
    int read_di_multi(uint32_t start_io_number, uint32_t end_io_number,
                      uint32_t* value) const;

    /* Read one cached digital output from the packed DO Bank. */
    int read_do(uint32_t io_number, bool* value) const;

    /* Read up to 32 cached digital outputs from the packed DO Bank. */
    int read_do_multi(uint32_t start_io_number, uint32_t end_io_number,
                      uint32_t* value) const;

    /* Write one cached digital output; PDO Shadow updates at the cycle flush. */
    int write_do(uint32_t io_number, bool value);

    /* Write up to 32 cached digital outputs with one packed DO Bank update. */
    int write_do_multi(uint32_t start_io_number, uint32_t end_io_number, uint32_t value);

    /* Read a PDO or virtual unscaled analog input value. */
    int read_ai(uint32_t io_number, uint32_t* value) const;

    /* Read a PDO Shadow or Virtual IO Bank analog output. */
    int read_ao(uint32_t io_number, uint32_t* value) const;

    /* Write a PDO Shadow or Virtual IO Bank analog output. */
    int write_ao(uint32_t io_number, uint32_t value);

    /* Inject one registered virtual digital-input value. */
    int write_virtual_di(uint32_t io_number, bool value);

    /* Inject one registered virtual analog-input value. */
    int write_virtual_ai(uint32_t io_number, uint32_t value);

    /* Clear PDO layout, aliases, IO Banks, Shadows, plans, and bindings. */
    void reset();

   private:
    /* Find an input PDO object. */
    const RtBusPdoCell* find_input(const IoObjectKey& key) const;

    /* Find an output PDO object. */
    const RtBusPdoCell* find_output(const IoObjectKey& key) const;

    struct Impl;
    Impl* impl_;
};

}  // namespace zmc

#endif  // _ZMC_IO_MANAGER_HPP_
