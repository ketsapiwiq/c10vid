import sys_bme680
import ucollections

# Import old module for compatibility
from sys_bme680 import *  # noqa

Bme680Data = ucollections.namedtuple(
    "Bme680Data", ["temperature", "humidity", "pressure", "gas_resistance"]
)

BSECData = ucollections.namedtuple(
    "BSECData",
    [
        "temperature",
        "humidity",
        "pressure",
        "gas_resistance",
        "iaq",
        "iaq_accuracy",
        "eco2",
        "breath_voc_equivalent",
    ],
)


class Bme680:
    """
    BME680 4-in-1 environmental sensor.

    **Example**:

    .. code-block:: python

        import bme680

        environment = bme680.Bme680()
        print("Current temperature: {:4.1f} °C".format(environment.temperature()))

    This class can also be used as a context-manager which will automatically
    deactivate the sensor on exit:

    .. code-block:: python

        import bme680

        with bme680.Bme680() as environment:
            print("H: {:4.1f}%".format(environment.humidity()))

        # Sensor is off again, saving power

    .. versionadded:: 1.10
    """

    def __init__(self):
        sys_bme680.init()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def get_data(self):
        """
        Get all sensor data at once.

        :py:meth:`~bme680.Bme680.get_data` returns a namedtuple with the
        following fields:

        - ``temperature``: Temperature in *°C*
        - ``humidity``: Relative humidity
        - ``pressure``: Barometric pressure in *hPa*
        - ``gas_resistance``: Gas resistance in *Ω*

        If the Bosch BSEC library is enabled in :ref:`card10_cfg`, the
        following additional fields will be returned:

        - ``iaq``: Indoor air quality indication
        - ``iaq_accuracy``: Accuracy of indoor air quality
        - ``eco2``: Equivalent CO2 content in *ppm*
        - ``breath_voc_equivalent``: Equivalent breath VOC in *ppm*

        If BSEC is enabled an instance of `bme680.BSECData` will be returned.
        If BSEC is not enabled an instance of `bme680.Bme680Data` will be returned.

        **Example**:

        .. code-block:: python

            import bme680

            with bme680.Bme680() as environment:
                data = environment.get_data()

                print("T: {}".format(data.temperature))
                print("H: {}".format(data.humidity))
        """

        try:
            return BSECData(*sys_bme680.bsec_get_data())
        except:
            return Bme680Data(*sys_bme680.get_data())

    def close(self):
        """
        Stop/deinit the BME680.

        If you no longer need measurements, you should call this function to
        save power.
        """
        sys_bme680.deinit()

    def temperature(self):
        """
        Measure current temperature in *°C*.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.temperature()))
        """
        return self.get_data().temperature

    def humidity(self):
        """
        Measure current relative humidity.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.humidity()))
        """
        return self.get_data().humidity

    def pressure(self):
        """
        Measure current barometric pressure in *hPa*.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.pressure()))
        """
        return self.get_data().pressure

    def gas_resistance(self):
        """
        Measure current gas resistance in *Ω*.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.gas_resistance()))
        """
        return self.get_data().gas_resistance

    def iaq(self):
        """
        Retrieve indoor air quality as defined by the Bosch BSEC library.

        BSEC needs to be enable in :ref:`card10_cfg`. Otherwise this method
        will raise an ``OSError`` exception.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.iaq()))

        .. versionadded:: 1.17
        """
        return BSECData(*sys_bme680.bsec_get_data()).iaq

    def iaq_accuracy(self):
        """
        Retrieve indoor air quality accuracy as defined by the Bosch BSEC library.

        BSEC needs to be enable in :ref:`card10_cfg`. Otherwise this method
        will raise an ``OSError`` exception.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.iaq_accuracy()))

        .. versionadded:: 1.17
        """

        return BSECData(*sys_bme680.bsec_get_data()).iaq_accuracy

    def eco2(self):
        """
        Retrieve equivalant CO2 as defined by the Bosch BSEC library.

        BSEC needs to be enable in :ref:`card10_cfg`. Otherwise this method
        will raise an ``OSError`` exception.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.eco2()))

        .. versionadded:: 1.17
        """

        return BSECData(*sys_bme680.bsec_get_data()).eco2

    def breath_voc_equivalent(self):
        """
        Retrieve equivalent breath VOC as defined by the Bosch BSEC library.

        BSEC needs to be enable in :ref:`card10_cfg`. Otherwise this method
        will raise an ``OSError`` exception.

        **Example**:

        .. code-block:: python

            environment = bme680.Bme680()
            print(str(environment.breath_voc()))

        .. versionadded:: 1.x
        """

        return BSECData(*sys_bme680.bsec_get_data()).breath_voc_equivalent
