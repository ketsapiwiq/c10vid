.. py:module:: bme680

``bme680`` - Environmental Sensor
=================================
Allows access to environmental data of card10's surroundings.

If ``bsec_enable`` is set in :ref:`card10_cfg`, the proprietary Bosch BSEC
library will be activated which offers the following extra functionality:

 - Manual temperature offset compensation
    The ``bsec_offset`` configuration allows to configure a static temperature
    offset. Please use a reference thermometer to determine the offset of your
    card10. If no offset is provided a default offset for a card10 which is
    connected to USB, has BLE active and is without a case is used.
 - A fixed measurement interval of 3 seconds
    This helps to stabilize the temperature of the card10.
 - An indoor air quality (IAQ)  and equivalent CO2 estimation algorithm
    Please refer to the :ref:`bsec_api` API documentation to get more information
    about how to interpret these estimates.

.. note::
   Please keep in mind that the BME680 can not directly measure CO2. It measures
   Volatile Organic Compounds (VOCs). The BSEC library uses this measurement
   to compute an Indoor Air Quality (IAQ) indication. It also assumes that all VOCs
   in the air are from human breath and computes an equivalent CO2 (eCO2)
   value from this. Please be aware of these facts when judging the accuracy
   of the IAQ and eCO2 values. Some more information can be found in the
   :ref:`bsec_api` API documentation.

.. warning::
   For the BSEC library to properly work the card10 should be kept running
   for at least 10 hours at least once. This is needed as the BSEC library
   periodically writes calibration information about the sensor to the
   card10's file system.

   Please make sure to observe the IAQ accuracy field. It will tell you if the
   IAQ and eCO2 measurements are deemed "accurate" by the BSEC library. Your
   application should either inform the user about the current accuracy (e.g.
   by color coding) or simply not show any values if the accuracy is below 2.


.. note::
   See also the BLE :ref:`ESS`.


**Example**:

.. code-block:: python

   import bme680, time

   with bme680.Bme680() as environment:
       while True:
           d = environment.get_data()

           print("Temperature:    {:10.2f} °C".format(d.temperature))
           print("Humidity:       {:10.2f} % r.h.".format(d.humidity))
           print("Pressure:       {:10.2f} hPa".format(d.pressure))
           print("Gas Resistance: {:10.2f} Ω".format(d.resistance))

           time.sleep(1)

You can use the return type of :py:meth:`~bme680.Bme680.get_data` to decide
if you want to use/display the additonal fields returned if BSEC is enabled.

.. code-block:: python

    if isinstance(d, bme680.BSECData):
        print("Air quality:    {:7d}".format(d.iaq))

Sensor Class
------------

.. autoclass:: bme680.Bme680
   :members:

Deprecated Interface
--------------------
The following functions should no longer be used directly.  The only exist for
compatibility as they were the old BME680 interface in previous firmware
versions.

.. py:function:: init()

   Initialize the sensor.

   Before being able to read data from the sensor, you have to call
   :py:func:`bme680.init`.

   .. versionadded:: 1.4
   .. deprecated:: 1.10
      Use the :py:class:`bme680.Bme680` class instead.

.. py:function:: get_data()

   Perform a single measurement of environmental data.

   :return: Tuple containing ``temperature`` (°C), ``humidity`` (% r.h.),
      ``pressure`` (hPa) and ``gas resistance`` (Ohm).

   .. versionadded:: 1.4
   .. deprecated:: 1.10
      Use the :py:class:`bme680.Bme680` class instead.

.. py:function:: deinit()

   Deinitialize the sensor.

   .. versionadded:: 1.4
   .. deprecated:: 1.10
      Use the :py:class:`bme680.Bme680` class instead.
