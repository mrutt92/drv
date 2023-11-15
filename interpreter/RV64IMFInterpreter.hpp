// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#ifndef RV64IMFINTERPRETER_HPP
#define RV64IMFINTERPRETER_HPP
#include "RISCVInstruction.hpp"
#include "RISCVHart.hpp"
#include "RV64IMInterpreter.hpp"
#include <stdexcept>
#include <boost/multiprecision/cpp_int.hpp>
#include <cmath>
#include <cfenv>

class RV64IMFInterpreter : public RV64IMInterpreter
{
public:
    RV64IMFInterpreter(): RV64IMInterpreter() {}

    /**
     * @brief temporarily changes rounding mode while in scope
     */
    struct RoundingModeGuard {
        RoundingModeGuard(int rm) {
            old_rounding_mode = fegetround();
            fesetround(rm);
        }
        ~RoundingModeGuard() {
            fesetround(old_rounding_mode);
        }
        int old_rounding_mode;
    };    
    

    void visitFMADD_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = std::fmaf(hart.sf(i.rs1()),
                                   hart.sf(i.rs2()),
                                   hart.sf(i.rs3()));
        hart.pc() += 4;
    }

    void visitFMSUB_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = std::fmaf(hart.sf(i.rs1()),
                                   hart.sf(i.rs2()),
                                   -hart.sf(i.rs3()));
        hart.pc() += 4;
    }

    void visitFNMSUB_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = std::fmaf(-hart.sf(i.rs1()),
                                   hart.sf(i.rs2()),
                                   hart.sf(i.rs3()));
        hart.pc() += 4;
    }

    void visitFNMADD_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = std::fmaf(-hart.sf(i.rs1()),
                                   hart.sf(i.rs2()),
                                   -hart.sf(i.rs3()));
        hart.pc() += 4;
    }

    void visitFADD_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = hart.sf(i.rs1()) + hart.sf(i.rs2());
        hart.pc() += 4;
    }

    void visitFSUB_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = hart.sf(i.rs1()) - hart.sf(i.rs2());
        hart.pc() += 4;
    }

    void visitFMUL_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = hart.sf(i.rs1()) * hart.sf(i.rs2());
        hart.pc() += 4;
    }

    void visitFDIV_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = hart.sf(i.rs1()) / hart.sf(i.rs2());
        hart.pc() += 4;
    }

    void visitFSQRT_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        hart.f(i.rd()) = std::sqrt(hart.sf(i.rs1()));
        hart.pc() += 4;
    }

    void visitFSGNJ_S(RISCVHart &hart, RISCVInstruction &i) override {
        hart.f(i.rd()) = std::copysignf(hart.sf(i.rs1()), hart.sf(i.rs2()));
        hart.pc() += 4;
    }

    void visitFSGNJN_S(RISCVHart &hart, RISCVInstruction &i) override {
        hart.f(i.rd()) = std::copysignf(hart.sf(i.rs1()), -hart.sf(i.rs2()));
        hart.pc() += 4;
    }

    void visitFSGNJX_S(RISCVHart &hart, RISCVInstruction &i) override {
        float s1 = std::copysignf(1.0, hart.sf(i.rs1()));
        float s2 = std::copysignf(1.0, hart.sf(i.rs2()));
        hart.f(i.rd()) = hart.sf(i.rs1()) * s2 * s1;
        hart.pc() += 4;
    }

    void visitFMIN_S(RISCVHart &hart, RISCVInstruction &i) override {
        hart.f(i.rd()) = std::fminf(hart.sf(i.rs1()), hart.sf(i.rs2()));
        hart.pc() += 4;
    }

    void visitFMAX_S(RISCVHart &hart, RISCVInstruction &i) override {
        hart.f(i.rd()) = std::fmaxf(hart.sf(i.rs1()), hart.sf(i.rs2()));
        hart.pc() += 4;
    }

    void visitFCVT_W_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        int32_t result = std::rintf(hart.sf(i.rs1()));
        hart.sx(i.rd()) = result;
        hart.pc() += 4;
    }

    void visitFCVT_WU_S_DYN(RISCVHart &hart, RISCVInstruction &i) override {
        RoundingModeGuard guard(hart.rm());
        uint32_t result = std::rintf(hart.sf(i.rs1()));
        hart.x(i.rd()) = result;
        hart.pc() += 4;
    }

};
#endif
