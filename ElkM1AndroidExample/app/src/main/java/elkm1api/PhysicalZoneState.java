/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package elkm1api;

public final class PhysicalZoneState {
  public final static PhysicalZoneState PZS_UNCONFIGURED = new PhysicalZoneState("PZS_UNCONFIGURED");
  public final static PhysicalZoneState PZS_OPEN = new PhysicalZoneState("PZS_OPEN");
  public final static PhysicalZoneState PZS_EOL = new PhysicalZoneState("PZS_EOL");
  public final static PhysicalZoneState PZS_SHORT = new PhysicalZoneState("PZS_SHORT");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static PhysicalZoneState swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + PhysicalZoneState.class + " with value " + swigValue);
  }

  private PhysicalZoneState(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private PhysicalZoneState(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private PhysicalZoneState(String swigName, PhysicalZoneState swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static PhysicalZoneState[] swigValues = { PZS_UNCONFIGURED, PZS_OPEN, PZS_EOL, PZS_SHORT };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

