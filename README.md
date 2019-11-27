# ArUcoRPi

## send_all_to_center

Pi only extracts coordinates from frame and sends them to center. The central PC does the aggregation and processing.

## aggregate_and_send

The Pi's aggregate the obtained coordinate data and send the coordinates in batch. The cntral PC does the processing.

## process_and_send

This branch is expected to be implemented once the system has been updated to rely on the pi's only for aggregation and processeing.
In such a setup, data can be directly sent to the trucks.
